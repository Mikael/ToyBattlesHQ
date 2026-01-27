# 4.2 Config setup for VPS
Assume that:
- I have a VPS whose public address is 199.222.32.3
- The VPS local IP is 123.456.78.9 (you can find this by opening a command prompt and using `ipconfig`)
- My client version is 1.1.1 (i.e. I did not modify it through the client tools as explained in the next chapters)
- The database inside the VPS is named microvolts-db
- My database username is root
- My database password is stored in the environment variable named "MV_DB_PW" (inside the VPS)
- My MariaDB runs on port 3305
- I do not have a website running (i.e. no admin panel)
- The VPS allows incoming connections to all the ports below through firewall settings
Then, here's a simple setup for a VPS:

```cpp
[AuthServer]
LocalIp = 123.456.78.9
Ip = 199.222.32.3
Port = 13000

[MainServer_1]
LocalIp = 123.456.78.9
Ip = 199.222.32.3
Port = 13005
IpcPort = 14005
IsPublic = true

[CastServer_1]
LocalIp = 123.456.78.9
Ip = 199.222.32.3
Port = 13006
IpcPort = 14006

[Database]
LocalIp = 123.456.78.9
Ip = 199.222.32.3
Port = 3305
DatabaseName = microvolts-db
Username = root
PasswordEnvironmentName = MV_DB_PW

[Website]
Ip = 199.222.32.3
Port = 8080

[Client]
ClientVersion = 0.0.3 # For ToyBattles Client in this repository; for original MV Surge client use 1.1.1
```


## Next
[4.3 Config setup for multiple servers across different VPSs](https://github.com/SoWeBegin/MicrovoltsEmulator/blob/mv1.1_2.0/doc/example_multiple_vps.md)
