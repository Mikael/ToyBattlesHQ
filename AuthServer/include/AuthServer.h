#ifndef AUTHENTICATION_SERVER_H
#define AUTHENTICATION_SERVER_H

#include <optional>
#include <memory>
#include <asio.hpp>
#include <string>
#include "DbPlayerInfo.h"
#include "AuthService.h"

namespace Auth
{
	using tcp = asio::ip::tcp;
	using ioContext = asio::io_context;

	class AuthServer
	{
	private:
		ioContext& m_io_context;
		tcp::acceptor m_acceptor;
		std::optional<tcp::socket> m_socket;
		Auth::Persistence::PersistentDatabase m_database;
		Auth::AuthService m_authService;

	public:
		AuthServer(ioContext& io_context, const std::string& ip, std::uint16_t port);
		void asyncAccept();
	};
}

#endif