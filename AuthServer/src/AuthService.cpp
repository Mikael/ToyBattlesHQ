#include "../include/AuthService.h"
#include <string>
#include "Utils.h"
#include "Utils/SetupParser.h"
#include "AuthUtils.h"
#include <libcppotp/auth.h>
#include <openssl/rand.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>

namespace Auth
{
	Auth::Enums::Login AuthService::authorizeGraded(const Auth::Structures::BasicAccountInfo& ainfo, const std::optional<std::string>& token,
		const std::string& plainPw, const std::string& plainIp, const std::string& plainHwid)
	{
		if (ainfo.secret.empty())
		{
			return Auth::Enums::Login::INCORRECT;
		}

		auto& authSetup = Common::Utils::SetupParser::getInstance().getAuthSetup();

		if (authSetup.enhancedSecurity)
		{ // Enhanced security mandates VPN tunnel and correct HWID for >= MOD grade

			asio::ip::network_v4 vpnNet = asio::ip::make_network_v4(authSetup.gradedAccessSubnet);
			asio::ip::address_v4 clientIp = asio::ip::make_address_v4(plainIp);

			if (plainIp.empty() || !isIpInSubnet(vpnNet, clientIp)) return Auth::Enums::Login::INCORRECT;
			if (plainHwid.empty()) return Auth::Enums::Login::INCORRECT;

			std::string dbGradedHash, dbGradedSalt;
			if (!m_persistentDatabase.getGradedHwid(ainfo.ainfoClient.accountId, dbGradedHash, dbGradedSalt))
			{
				return Auth::Enums::Login::INCORRECT;
			}

			if (dbGradedHash.empty() || dbGradedSalt.empty())
			{ // on first graded login, register "permanent" HWID
				dbGradedSalt = generateRandomSalt();
				dbGradedHash = hashHwid(plainHwid, dbGradedSalt);
				if (!m_persistentDatabase.setGradedHwid(ainfo.ainfoClient.accountId, dbGradedHash, dbGradedSalt))
				{
					return Auth::Enums::Login::INCORRECT;
				}
			}
			else
			{
				if (hashHwid(plainHwid, dbGradedSalt) != dbGradedHash) return Auth::Enums::Login::INCORRECT;
			}

			// Update "current" HWID
			std::string currentSalt = generateRandomSalt();
			std::string currentHwidHash = hashHwid(plainHwid, currentSalt);
			if (!m_persistentDatabase.updateCurrentHwid(ainfo.ainfoClient.accountId, currentHwidHash, currentSalt))
			{
				return Auth::Enums::Login::INCORRECT;
			}
		}

		const std::string hashedPassword = Common::Utils::calculateHashCryptoPP<CryptoPP::SHA256>(plainPw);
		const bool passwordOk = hashedPassword == ainfo.hashedPassword;

		bool tokenOk = verifyToken(ainfo.secret, token);

		auto& counters = m_badLoginAttempts[ainfo.ainfoClient.accountId];
		if (!passwordOk) ++counters.totalWrongPasswords;
		if (!tokenOk)    ++counters.totalWrong2fas;

		constexpr std::uint32_t MAX_FAILED_ATTEMPTS = 5;
		if (counters.totalWrongPasswords >= MAX_FAILED_ATTEMPTS || counters.totalWrong2fas >= MAX_FAILED_ATTEMPTS)
		{
			if (!tryLockAccount(ainfo.ainfoClient.accountId))
			{
				return Auth::Enums::Login::INCORRECT;
			}

			m_badLoginAttempts.erase(ainfo.ainfoClient.accountId);
			return Auth::Enums::Login::INCORRECT;
		}

		if (!passwordOk || !tokenOk)
		{
			return Auth::Enums::Login::INCORRECT;
		}

		m_badLoginAttempts.erase(ainfo.ainfoClient.accountId);

		const std::string currentTime = std::format("{:%Y-%m-%d %X}", std::chrono::utc_clock::now());
		if (ainfo.suspendedUntil > currentTime)
		{
			return Auth::Enums::Login::SUSPENDED;
		}

		return Auth::Enums::Login::SUCCESS;
	}

	Auth::Enums::Login AuthService::authorizeUngraded(const Auth::Structures::BasicAccountInfo& ainfo, const std::optional<std::string>& token, const std::string& plainPw)
	{
		const bool enhancedSecurity = Common::Utils::SetupParser::getInstance().getAuthSetup().enhancedSecurity;
		const std::string hashedPassword = Common::Utils::calculateHashCryptoPP<CryptoPP::SHA256>(plainPw);
		const bool passwordOk = hashedPassword == ainfo.hashedPassword;

		bool tokenOk = true;
		if (ainfo.secret.empty())
		{
			if (enhancedSecurity)
			{ // enhancedSecurity enabled mandates 2FA for everyone
				return Auth::Enums::Login::INCORRECT; 
			}
		}
		else
		{
			tokenOk = verifyToken(ainfo.secret, token);
		}

		auto& counters = m_badLoginAttempts[ainfo.ainfoClient.accountId];
		if (!passwordOk) ++counters.totalWrongPasswords;
		if (!tokenOk)    ++counters.totalWrong2fas;

		constexpr std::uint32_t MAX_2FA_ATTEMPTS = 5;
		if (counters.totalWrong2fas >= MAX_2FA_ATTEMPTS)
		{
			if (!tryLockAccount(ainfo.ainfoClient.accountId))
			{
				return Auth::Enums::Login::INCORRECT;
			}

			m_badLoginAttempts.erase(ainfo.ainfoClient.accountId);
			return Auth::Enums::Login::INCORRECT; // On purpose: don't let an attacker know that we banned the account, otherwise they know they used the correct 2FA + password
		}

		if (!passwordOk || !tokenOk)
		{
			return Auth::Enums::Login::INCORRECT;
		}

		m_badLoginAttempts.erase(ainfo.ainfoClient.accountId);

		const std::string currentTime = std::format("{:%Y-%m-%d %X}", std::chrono::utc_clock::now());

		if (ainfo.suspendedUntil > currentTime)
		{
			return Auth::Enums::Login::SUSPENDED;
		}

		return Auth::Enums::Login::SUCCESS;
	}

	std::string AuthService::hashHwid(const std::string& hwid, const std::string& salt) const
	{
		std::string concatenated = hwid + salt;
		std::string digest;

		CryptoPP::SHA256 hash;
		CryptoPP::StringSource ss(concatenated, true, new CryptoPP::HashFilter(hash, new CryptoPP::HexEncoder(new CryptoPP::StringSink(digest), false)));

		return digest;
	}

	std::string AuthService::generateRandomSalt(std::size_t length) const
	{
		CryptoPP::AutoSeededRandomPool rng;
		std::string saltBytes(length, 0);
		rng.GenerateBlock(reinterpret_cast<CryptoPP::byte*>(&saltBytes[0]), saltBytes.size());

		std::string hex;
		CryptoPP::HexEncoder encoder;
		encoder.Attach(new CryptoPP::StringSink(hex));
		encoder.Put(reinterpret_cast<const CryptoPP::byte*>(saltBytes.data()), saltBytes.size());
		encoder.MessageEnd();

		return hex; 
	}

	bool AuthService::isIpInSubnet(const asio::ip::network_v4& network, const asio::ip::address_v4& ip) const
	{
		asio::ip::network_v4 test_network(ip, network.prefix_length());
		return network.canonical().address() == test_network.canonical().address();
	}

	bool AuthService::verifyToken(const std::string& secret, const std::optional<std::string>& token)
	{
		if (secret.empty() || !token.has_value()) return false;

		const int t_interval = 30;
		std::time_t now = std::time(nullptr);

		for (int i = -1; i <= 1; ++i)
		{
			auto expectedToken = auth::generateToken(secret, now + i * t_interval, t_interval);
			std::ostringstream oss;
			oss << std::setw(6) << std::setfill('0') << expectedToken;

			if (oss.str() == token.value()) return true;
		}

		return false;
	}

	bool AuthService::tryLockAccount(std::uint32_t accountId) 
	{
		constexpr int MAX_RETRIES = 3;
		bool locked = false;
		for (int i = 0; i < MAX_RETRIES; ++i)
		{
			locked = m_persistentDatabase.removeGradeAndSuspend(accountId);
			if (locked) break;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		return locked;
	}

	std::uint32_t AuthService::generateAccountKey() const
	{
		std::uint32_t value;
		if (RAND_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value)) != 1) {
			// entropy source not available => no cryptographically secure random bytes generated
			return 0; // account key 0 is default, main server won't accept it
		}
		return value;
	}

	Auth::Structures::Parsed2faLogin AuthService::parseUsernameAnd2FA(const std::string& rawUsername) const
	{
		Auth::Structures::Parsed2faLogin res{};
		const auto pos = rawUsername.find('+');

		if (pos == std::string::npos)
		{
			res.username = rawUsername;
			return res;
		}

		res.username = rawUsername.substr(0, pos);
		res.token = rawUsername.substr(pos + 1);

		if (res.token->size() != 6 || !std::all_of(res.token->begin(), res.token->end(), ::isdigit))
		{
			res.token->clear(); 
		}

		return res;
	}


	std::expected<Auth::Structures::BasicAccountInfo, Auth::Enums::Login>
		AuthService::login(const std::string& username, const std::string& password, const std::string& plainIp, const std::string& plainHwid)
	{
		const auto parsed = parseUsernameAnd2FA(username);
		const auto result = m_persistentDatabase.getCompletePlayerInfo(parsed.username);
		if (!result) return std::unexpected(result.error());

		Auth::Structures::BasicAccountInfo userInfo = result.value();
		Auth::Enums::Login authResult = (userInfo.grade >= 3)
			? authorizeGraded(userInfo, parsed.token, password, plainIp, plainHwid)
			: authorizeUngraded(userInfo, parsed.token, password);

		if (authResult != Auth::Enums::SUCCESS)
		{
			m_persistentDatabase.addHash(userInfo.ainfoClient.accountId, 0); // AccountKey=0 signals failure to MainServer
			return std::unexpected(authResult);
		}

		userInfo.ainfoClient.hashKey = generateAccountKey();
		m_persistentDatabase.addHash(userInfo.ainfoClient.accountId, userInfo.ainfoClient.hashKey);
		m_persistentDatabase.updateLastLoggedNow(userInfo.ainfoClient.accountId, plainIp); // TODO: hash this
		return userInfo;
	}
}