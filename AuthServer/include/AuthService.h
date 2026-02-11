#ifndef AUTH_SERVICE_HEADER
#define AUTH_SERVICE_HEADER

#include "DbPlayerInfo.h"
#include <expected>
#include "AuthEnums.h"
#include "Structures/AuthAccountInfo.h"
#include <unordered_map>
#include <optional>
#include "asio.hpp"

namespace Auth
{
	class AuthService
	{
	private:
		Auth::Persistence::PersistentDatabase m_persistentDatabase;
		std::unordered_map<std::uint32_t, Auth::Structures::LoginWrongAttempts> m_badLoginAttempts; // [aid] [LoginWrongAttempts]

	public:
		std::expected<Auth::Structures::BasicAccountInfo, Auth::Enums::Login> login(const std::string& username, const std::string& password, const std::string& ip,
			const std::string& hwid);

	private:
		Auth::Structures::Parsed2faLogin parseUsernameAnd2FA(const std::string& rawUsername) const;
		Auth::Enums::Login authorizeGraded(const Auth::Structures::BasicAccountInfo& ainfo, const std::optional<std::string>& token, const std::string& clientPw,
			const std::string& ip, const std::string& hwid);
		Auth::Enums::Login authorizeUngraded(const Auth::Structures::BasicAccountInfo& ainfo, const std::optional<std::string>& token, const std::string& clientPw);

		bool verifyToken(const std::string& secret, const std::optional<std::string>& token);
		bool tryLockAccount(std::uint32_t accountId);
		std::uint32_t generateAccountKey() const;
		bool isIpInSubnet(const asio::ip::network_v4& network, const asio::ip::address_v4& ip) const;
		std::string generateRandomSalt(std::size_t length = 16) const;
		std::string hashHwid(const std::string& hwid, const std::string& salt) const;
	};
}

#endif
