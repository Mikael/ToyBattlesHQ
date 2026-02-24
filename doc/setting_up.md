# 3.2 Setting up the emulator
Starting from version 2.0, setting up the emulator should be much easier than before.

A new `config.ini` file was now added inside the `Setup` folder (in the root directory). 

## config.ini Configuration Guide
This file contains all the configuration values used by the servers (Auth, Main, Cast) and supporting components (database, client, website). Here's how to fill it out correctly:
Note: the file is inside the `Setup` folder. It should already contain most of the things that will enable it to work for localhost (`127.0.0.1`).

### [AuthServer]
This section configures your authentication server, which handles player logins.

**LocalIp**: Your machine’s local IPv4 address (e.g., 192.168.x.x). To find it, use `ipconfig` on a terminal. (Note: if you have services like RadminVPN, you may have to use their IP). 

**Ip**: Your public IP or just 127.0.0.1 if everything runs locally.

**Port**: The port used by the Auth Server, default is 13000.

### [MainServer_N]
Examples: MainServer_1, MainServer_2, ..., up to MainServer_9

You can run multiple Main Servers, each handling player data, inventory, shops, trades, rooms, and more.

**LocalIp**: Local IPv4 address of the machine it's running on. This is needed to differentiate multiple servers running in different machines.

**Ip**: Public IP or localhost (127.0.0.1)

**Port**: Port the server listens on (e.g., default 13005)

**IpcPort**: Port used for internal server communication

**IsPublic**: Set this to true if this server should be accessible to non-graded players from the channel list

### [CastServer_N]
Same as above, from CastServer_1 to CastServer_9
These handle all in-match/gameplay logic like damage, position updates, match rules, and more.

**LocalIp**: Local IP address

**Ip**: Public or localhost IP (127.0.0.1)

**Port**: Gameplay traffic port (e.g., default 13006)

**IpcPort**: Used for inter-server communication

### [Database]
Connection settings for your MariaDB database.

**LocalIp**: Use your local Ipv4 as before.

**Ip**: Ip where the database is accessed, for example 127.0.0.1

**Port**: Port of the DB (commonly 3306, here it's 3305, and I will provide a `my.ini` file in the next chapter that uses `3305`)

**DatabaseName**: Name of the database (e.g., microvolts-db)

**Username**: Database user (e.g., root)

**PasswordEnvironmentName:** Name of the environment variable (not the actual password) that holds the DB password

### [Website]
Defines the API endpoint used for admin panel or external requests.

**Ip**: IP of the admin panel backend (often 127.0.0.1).

**Port**: Port your panel or API runs on (e.g., 8080).

### [Client]
Used for version checking during client login.

**ClientVersion**: The version string expected by the servers (e.g., 0.0.3 for the ToyBattles Client).
#### Important: 
The servers check the ClientVersion. If you specify a client version (e.g. 1.1.1) but your client is actually using a different version than what you specified, you will **not be able to login** on the main servers with **non-graded accounts**. At ToyBattles we use this to force players to use the most recently updated client.

Graded accounts don't have this limitation, this allows graded accounts to enter the server even with different clients, for example for testing purposes.

Note that for ToyBattles Client (on the Release section on this Repository), the client version is 0.0.3. If you decide to use the original Microvolts Surge client (although discouraged) you will need to use version 1.1.1.

# ⚠️ Notes & Warnings
You can configure up to 9 Main Servers and 9 Cast Servers.

If anything is misconfigured (invalid IPs, missing ports, database issues, etc.), the server will print an error message on startup, so always check the console output when launching.

**Make sure that the ports you use for the emulator aren't used by other services running in your OS.**

I’ll also provide example setup.ini files for different setups in later chapters:

- Single local server (for testing everything on one machine)
- Single VPS server
- Multiple regional VPS servers (e.g., NA, EU, ASIA)


## Next
[3.2.1 Enhanced Security](https://github.com/SoWeBegin/ToyBattlesHQ/blob/toybattles_mvsurge/doc/enhanced_security.md)

