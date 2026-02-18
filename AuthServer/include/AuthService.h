#ifndef AUTH_SERVICE_HEADER
#define AUTH_SERVICE_HEADER

#include "DbPlayerInfo.h"
#include <expected>
#include "AuthEnums.h"
#include "Structures/AuthAccountInfo.h"
#include <unordered_map>
#include <optional>
#include "asio.hpp"
#include <EmailDispatcher.h>
#include <libcppotp/auth.h>

namespace Auth
{
	class AuthService
	{
	private:
		Auth::Persistence::PersistentDatabase& m_persistentDatabase;
		Common::Utils::EmailDispatcher& m_emailDispatcher;
		std::unordered_map<std::uint32_t, Auth::Structures::LoginWrongAttempts> m_badLoginAttempts; // [aid] [LoginWrongAttempts]

	public:
		AuthService(Auth::Persistence::PersistentDatabase& persistentDatabase, Common::Utils::EmailDispatcher& emailDispatcher)
			: m_persistentDatabase{ persistentDatabase }
			, m_emailDispatcher{ emailDispatcher }
		{
		}

		std::expected<Auth::Structures::BasicAccountInfo, Auth::Enums::Login> login(const std::string& username, const std::string& password, const std::string& ip,
			std::uint_least16_t port, const std::string& hwid);

	private:
		Auth::Structures::Parsed2faLogin parseUsernameAnd2FA(const std::string& rawUsername) const;
		Auth::Enums::Login authorizeGraded(const Auth::Structures::BasicAccountInfo& ainfo, const std::optional<std::string>& token,
			const std::string& plainPw, const std::string& plainIp, const std::string& plainHwid, std::uint_least16_t port);
		Auth::Enums::Login authorizeUngraded(const Auth::Structures::BasicAccountInfo& ainfo, const std::optional<std::string>& token, const std::string& clientPw);

		bool tryLockAccount(std::uint32_t accountId, bool isGraded);
		std::uint32_t generateAccountKey() const;
		std::string generateRandomSalt(std::size_t length = 16) const;

		bool verifyToken(const std::string& encryptedSecret, const std::optional<std::string>& token);
	};
}

#endif
