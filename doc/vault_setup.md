## Linux Setup
### Prerequisites
```cpp
sudo apt update && sudo apt upgrade -y
sudo apt install -y openssl unzip curl ufw
```

### 1. Install Vault
```cpp
cd /tmp
wget https://releases.hashicorp.com/vault/1.15.6/vault_1.15.6_linux_amd64.zip
unzip vault_1.15.6_linux_amd64.zip
sudo mv vault /usr/local/bin/
sudo chmod 755 /usr/local/bin/vault

vault --version // for verification only
```

### 2. Create folder structure
```cpp
mkdir -p ~/vault/{config,data,tls,logs,backups}
chmod 700 ~/vault ~/vault/data
```

### 3. Generate TLS certificates (via SAN)
```cpp
cd ~/vault/tls

// For the following, make sure to update it with your info:
cat > openssl-san.cnf << 'EOF'
[ req ]
default_bits       = 2048
prompt             = no
default_md         = sha256
req_extensions     = req_ext
distinguished_name = dn

[ dn ]
C=US
ST=State
L=City
O=Company
OU=IT
CN= VAULT_VPS_IP_HERE

[ req_ext ]
subjectAltName = @alt_names

[ alt_names ]
IP.1 = VAULT_VPS_IP_HERE
IP.2 = 127.0.0.1
DNS.1 = localhost
EOF

// Generate keys, CSR and self signed certificates and PFX for windows clients:
openssl genrsa -out vault.key 2048
chmod 600 vault.key
openssl req -new -key vault.key -out vault.csr -config openssl-san.cnf

openssl x509 -req \
    -in vault.csr \
    -signkey vault.key \
    -out vault.crt \
    -days 3650 \
    -extensions req_ext \
    -extfile openssl-san.cnf
    
openssl pkcs12 -export -out vault.pfx -inkey vault.key -in vault.crt
// This will ask for an export password, store it somewhere securely
```

### 4. Install the certificate on all VPSs
On vault VPS:
```cpp
sudo cp ~/vault/tls/vault.crt /usr/local/share/ca-certificates/vault.crt
sudo update-ca-certificates
```
From other VPSs (assuming they're all linux):
```cpp
// Here use correct username/IP
scp username@vault-server-ip:~/vault/tls/vault.crt /tmp/    
sudo cp /tmp/vault.crt /usr/local/share/ca-certificates/vault.crt
sudo update-ca-certificates
```

### 5. Create Vault config
```cpp
cd ~/vault/config

// Use your paths here
sudo nano config.hcl 
ui = true
disable_mlock = true

storage "file" {
  path = "PATH_TO_VAULT_DATA_FOLDER"
}

listener "tcp" {
  address       = "0.0.0.0:8200"
  tls_cert_file = "PATH_TO_VAULT_TLS_FOLDER/vault.crt"
  tls_key_file  = "PATH_TO_VAULT_TLS_FOLDER/vault.key"
}

api_addr = "https://YOUR_VPS_IP:8200"
```

### 6. (Optional) Set environment variables
```cpp
echo 'export VAULT_ADDR="https://127.0.0.1:8200"' >> ~/.bashrc
echo 'export VAULT_CACERT="PATH_TO_VAULT_TLS_FOLDER/vault.crt"' >> ~/.bashrc
source ~/.bashrc
```

### 7. Test vault & initialize
Note: Save the generated tokens & unseal keys inside vault/backups
```cpp
// Start manually
vault server -config=<PATH_TO_VAULT_CONFIG_FOLDER>/config.hcl

// test if connection works (via another cmd)
curl --cacert PATH_TO_VAULT_TLS_FOLDER/vault.crt https://127.0.0.1:8200/v1/sys/health

// Initialize -- SAVE THE GENERATED "Initial Root Token" and "Unseal Key"
vault operator init -key-shares=1 -key-threshold=1

// Unseal vault
vault operator unseal <YOUR_UNSEAL_KEY_HERE>

// Login
vault login <INITIAL-ROOT-TOKEN>

// Status
vault status

// Now take the above generated keys and save them in vault/backup
echo "YOUR_UNSEAL_KEY" > VAULT_BACKUPS_FOLDER/unseal-key.txt
echo "YOUR_ROOT_TOKEN" > VAULT_BACKUPS_FOLDER/root-token.txt
chmod 600 ~/vault/backups/*.txt
```

### 8. Make Vault restart on failure & reboot
First of all, stop Vault (since we started it before).
`pkill vault`
Then:
```cpp
sudo nano /etc/systemd/system/vault.service

// Copy this, adjust paths and IPs:
[Unit]
Description=Vault Server
Documentation=https://www.vaultproject.io/docs/
Requires=network-online.target
After=network-online.target
ConditionFileNotEmpty=PATH_TO_VAULT_CONFIG_FOLDER/config.hcl

[Service]
User=YOUR_USERNAME
Group=YOUR_USERNAME
PIDFile=PATH_TO_VAULT_LOGS_FOLDER/vault.pid
ExecStart=/usr/local/bin/vault server -config=PATH_TO_VAULT_CONFIG_FOLDER/config.hcl
ExecReload=/bin/kill -HUP $MAINPID
KillMode=process
KillSignal=SIGINT
Restart=on-failure
RestartSec=5
TimeoutStopSec=30
StartLimitBurst=3
LimitMEMLOCK=infinity
LimitNOFILE=65536

PrivateTmp=yes
ProtectSystem=full
ProtectHome=read-only
NoNewPrivileges=yes

[Install]
WantedBy=multi-user.target

// Create the log file:
touch PATH_TO_VAULT_LOG_FOLDER/vault.log
chmod 640 ~/vault/logs/vault.log

// Enable it
sudo systemctl daemon-reload
sudo systemctl enable vault
sudo systemctl start vault

// Status and logs:
sudo systemctl status vault
sudo journalctl -u vault -f
```

### 9. Configure Firewall
```cpp
// of course, allow SSH
sudo ufw allow 22/tcp

// Allow Vault access only from specific VPS IPs (or your own PC), example:
sudo ufw allow from 1.2.3.4 to any port 8200 proto tcp  

// Then enable the new rules 
sudo ufw --force enable
sudo ufw status verbose
```

### 10. Auto unseal (discouraged, but simple)
We have stored the tokens and secrets directly in the backups, but generally you want to store them somewhere securely (eg outside the VPS where Vault runs).
But this setup allows us to auto unseal automatically whenever vault crashes or the VPS is restarted.
```cpp
sudo nano ~/vault/auto-unseal.sh 

// Then, copy paste this script - update your paths
#!/bin/bash
export VAULT_ADDR="https://127.0.0.1:8200"
export VAULT_CACERT="PATH_TO_VAULT_TLS_FOLDER/vault.crt"
sleep 10  
if /usr/local/bin/vault status | grep -q "Sealed.*true"; then
    /usr/local/bin/vault operator unseal $(cat PATH_TO_VAULT_BACKUPS_FOLDER/unseal-key.txt)
    echo "Vault unsealed at $(date)" >> PATH_TO_VAULT_LOGS_FOLDER/unseal.log
else
    echo "Vault already unsealed at $(date)" >> PATH_TO_VAULT_LOGS_FOLDER/unseal.log
fi

// Then:
chmod +x PATH_TO_VAULT/auto-unseal.sh

// Create systemd service so the script runs for auto unsteal:
sudo nano /etc/systemd/system/vault-unseal.service

// Then copy paste this (replace with your paths/usernames)
[Unit]
Description=Vault Auto-Unseal
After=vault.service
Requires=vault.service

[Service]
Type=oneshot
User=YOUR_USERNAME
Group=YOUR_USERNAME
ExecStart=PATH_TO_VAULT_FOLDER/auto-unseal.sh
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target

// Then reload and enable
sudo systemctl daemon-reload
sudo systemctl enable vault-unseal.service
