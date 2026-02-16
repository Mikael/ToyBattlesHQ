
DROP TABLE IF EXISTS `blockedplayers`;
CREATE TABLE `blockedplayers` (
  `AccountID` int(11) NOT NULL,
  `TargetAccountID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
LOCK TABLES `blockedplayers` WRITE;
UNLOCK TABLES;



DROP TABLE IF EXISTS `capsuleevents`;
CREATE TABLE `capsuleevents` (
  `StartDate` datetime NOT NULL,
  `EndDate` datetime NOT NULL,
  `NewMpPrice` int(11) NOT NULL,
  `NewRtPrice` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;
LOCK TABLES `capsuleevents` WRITE;
INSERT INTO `capsuleevents` VALUES ('1970-01-01 01:00:00','1970-01-01 01:00:00',0,0);
UNLOCK TABLES;

DROP TABLE IF EXISTS `chatlogs`;
CREATE TABLE `chatlogs` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `AccountID` int(10) unsigned NOT NULL,
  `Message` longtext DEFAULT NULL,
  `Date` datetime DEFAULT current_timestamp(),
  `Processed` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `idx_date` (`Date`),
  KEY `idx_account` (`AccountID`,`Date`),
  KEY `idx_unprocessed` (`Processed`,`Date`)
) ENGINE=InnoDB AUTO_INCREMENT=2965379 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;

LOCK TABLES `chatlogs` WRITE;
UNLOCK TABLES;


DROP TABLE IF EXISTS `cheatflags`;
CREATE TABLE `cheatflags` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `Description` text NOT NULL,
  `Timestamp` datetime NOT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=110 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;
LOCK TABLES `cheatflags` WRITE;
UNLOCK TABLES;


DROP TABLE IF EXISTS `clans`;
CREATE TABLE `clans` (
  `ClanId` int(11) NOT NULL,
  `Clanname` varchar(17) NOT NULL,
  `ClanFrontIcon` int(11) NOT NULL,
  `ClanBackIcon` int(11) NOT NULL,
  `TotalContribution` int(11) DEFAULT 0,
  `TotalWins` int(11) DEFAULT 0,
  `TotalLosses` int(11) DEFAULT 0,
  `TotalDraws` int(11) DEFAULT 0,
  `CreatedDate` date DEFAULT curdate(),
  `last_match_time` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;


LOCK TABLES `clans` WRITE;
UNLOCK TABLES;


DROP TABLE IF EXISTS `eventcommands`;
CREATE TABLE `eventcommands` (
  `ExpirationDate` datetime NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;
LOCK TABLES `eventcommands` WRITE;
UNLOCK TABLES;


DROP TABLE IF EXISTS `eventmissions`;
CREATE TABLE `eventmissions` (
  `AccountID` int(11) NOT NULL,
  `TotalMission1` int(11) DEFAULT 0,
  `TotalMission2` int(11) DEFAULT 0,
  `TotalMission3` int(11) DEFAULT 0,
  `TotalMission4` int(11) DEFAULT 0,
  `TotalMission5` int(11) DEFAULT 0,
  PRIMARY KEY (`AccountID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;

LOCK TABLES `eventmissions` WRITE;
UNLOCK TABLES;

--
-- Table structure for table `eventmissionsinfo`
--

DROP TABLE IF EXISTS `eventmissionsinfo`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `eventmissionsinfo` (
  `StartDate` datetime NOT NULL,
  `EndDate` datetime NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `eventmissionsinfo`
--

LOCK TABLES `eventmissionsinfo` WRITE;
/*!40000 ALTER TABLE `eventmissionsinfo` DISABLE KEYS */;
INSERT INTO `eventmissionsinfo` VALUES ('1970-01-01 01:00:00','1970-01-01 01:00:00');
/*!40000 ALTER TABLE `eventmissionsinfo` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `eventsmaps`
--

DROP TABLE IF EXISTS `eventsmaps`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `eventsmaps` (
  `GameMap` int(11) NOT NULL,
  `StartDate` datetime NOT NULL,
  `EndDate` datetime NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `eventsmaps`
--

LOCK TABLES `eventsmaps` WRITE;
/*!40000 ALTER TABLE `eventsmaps` DISABLE KEYS */;
/*!40000 ALTER TABLE `eventsmaps` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `eventsmodes`
--

DROP TABLE IF EXISTS `eventsmodes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `eventsmodes` (
  `GameMode` int(11) NOT NULL,
  `StartDate` datetime NOT NULL,
  `EndDate` datetime DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `eventsmodes`
--

LOCK TABLES `eventsmodes` WRITE;
/*!40000 ALTER TABLE `eventsmodes` DISABLE KEYS */;
/*!40000 ALTER TABLE `eventsmodes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `expmpbonusevents`
--

DROP TABLE IF EXISTS `expmpbonusevents`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `expmpbonusevents` (
  `StartDate` datetime NOT NULL,
  `EndDate` datetime NOT NULL,
  `ExpBonusPercent` int(11) NOT NULL,
  `MpBonusPercent` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `expmpbonusevents`
--

LOCK TABLES `expmpbonusevents` WRITE;
/*!40000 ALTER TABLE `expmpbonusevents` DISABLE KEYS */;
INSERT INTO `expmpbonusevents` VALUES ('1970-01-01 01:00:00','1970-01-01 01:00:00',0,0);
/*!40000 ALTER TABLE `expmpbonusevents` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `friendlist`
--

DROP TABLE IF EXISTS `friendlist`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `friendlist` (
  `AccountID` int(11) NOT NULL,
  `TargetAccountID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `friendlist`
--

LOCK TABLES `friendlist` WRITE;
/*!40000 ALTER TABLE `friendlist` DISABLE KEYS */;
/*!40000 ALTER TABLE `friendlist` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `gamelogs`
--

DROP TABLE IF EXISTS `gamelogs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `gamelogs` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `LogType` varchar(255) NOT NULL,
  `Message` text NOT NULL,
  `Severity` varchar(20) NOT NULL,
  `CreatedAt` timestamp NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=113 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `gamelogs`
--

LOCK TABLES `gamelogs` WRITE;
/*!40000 ALTER TABLE `gamelogs` DISABLE KEYS */;
/*!40000 ALTER TABLE `gamelogs` ENABLE KEYS */;
UNLOCK TABLES;


DROP TABLE IF EXISTS `giftbox`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `giftbox` (
  `accountId` int(11) NOT NULL,
  `timestamp` int(11) NOT NULL,
  `itemId` int(11) NOT NULL,
  `sender` varchar(17) NOT NULL,
  `message` varchar(300) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `giftbox`
--

LOCK TABLES `giftbox` WRITE;
/*!40000 ALTER TABLE `giftbox` DISABLE KEYS */;
/*!40000 ALTER TABLE `giftbox` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `itemlogs`
--

DROP TABLE IF EXISTS `itemlogs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `itemlogs` (
  `AccountID` bigint(20) unsigned NOT NULL,
  `Date` datetime NOT NULL,
  `ItemNumber` bigint(20) unsigned NOT NULL,
  `ItemID` bigint(20) unsigned NOT NULL,
  `Action` varchar(2000) DEFAULT NULL,
  `ExpirationDate` int(10) unsigned DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `itemlogs`
--

LOCK TABLES `itemlogs` WRITE;
/*!40000 ALTER TABLE `itemlogs` DISABLE KEYS */;
/*!40000 ALTER TABLE `itemlogs` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `mailbox`
--

DROP TABLE IF EXISTS `mailbox`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `mailbox` (
  `accountId` int(11) NOT NULL,
  `timestamp` int(11) NOT NULL,
  `uniqueId` text DEFAULT NULL,
  `nickname` varchar(17) NOT NULL,
  `message` varchar(300) NOT NULL,
  `sent` int(11) NOT NULL,
  `isNew` int(11) DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `mailbox`
--

LOCK TABLES `mailbox` WRITE;
/*!40000 ALTER TABLE `mailbox` DISABLE KEYS */;
/*!40000 ALTER TABLE `mailbox` ENABLE KEYS */;
UNLOCK TABLES;



DROP TABLE IF EXISTS `monthlyrewards`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `monthlyrewards` (
  `ItemID` int(11) NOT NULL,
  `Date` varchar(30) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `monthlyrewards`
--

LOCK TABLES `monthlyrewards` WRITE;
/*!40000 ALTER TABLE `monthlyrewards` DISABLE KEYS */;
INSERT INTO `monthlyrewards` VALUES (4308102,'2026-02-09'),(5336544,'2026-02-09'),(5336544,'2026-02-09'),(5336544,'2026-02-09'),(5336542,'2026-02-09'),(4305005,'2026-02-09'),(4305006,'2026-02-09'),(5336544,'2026-02-09'),(4305005,'2026-02-09'),(4305005,'2026-02-09'),(4305006,'2026-02-09'),(5336542,'2026-02-09'),(5336544,'2026-02-09'),(4308102,'2026-02-09'),(5336542,'2026-02-09'),(4305006,'2026-02-09'),(5336542,'2026-02-09'),(4308101,'2026-02-09'),(4305006,'2026-02-09'),(4308101,'2026-02-09'),(5336544,'2026-02-09'),(4308101,'2026-02-09'),(5336544,'2026-02-09'),(4308102,'2026-02-09'),(4305005,'2026-02-09'),(5336544,'2026-02-09'),(5336544,'2026-02-09'),(5336542,'2026-02-09'),(5336542,'2026-02-09'),(5336544,'2026-02-09'),(5336544,'2026-02-09'),(4305006,'2026-02-09');
/*!40000 ALTER TABLE `monthlyrewards` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `pendingfriendrequests`
--

DROP TABLE IF EXISTS `pendingfriendrequests`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `pendingfriendrequests` (
  `AccountID` int(11) NOT NULL,
  `TargetAccountID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `pendingfriendrequests`
--

LOCK TABLES `pendingfriendrequests` WRITE;
/*!40000 ALTER TABLE `pendingfriendrequests` DISABLE KEYS */;
/*!40000 ALTER TABLE `pendingfriendrequests` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tradeevents`
--

DROP TABLE IF EXISTS `tradeevents`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tradeevents` (
  `StartDate` datetime DEFAULT NULL,
  `EndDate` datetime DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tradeevents`
--

LOCK TABLES `tradeevents` WRITE;
/*!40000 ALTER TABLE `tradeevents` DISABLE KEYS */;
INSERT INTO `tradeevents` VALUES ('1970-01-01 01:00:00','1970-01-01 01:00:00');
/*!40000 ALTER TABLE `tradeevents` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `userachievements`
--

DROP TABLE IF EXISTS `userachievements`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `userachievements` (
  `AccountID` int(11) DEFAULT NULL,
  `AchievementIndex` int(11) DEFAULT NULL,
  `AchievementType` int(11) DEFAULT NULL,
  UNIQUE KEY `unique_achievement` (`AccountID`,`AchievementIndex`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `userachievements`
--

LOCK TABLES `userachievements` WRITE;
/*!40000 ALTER TABLE `userachievements` DISABLE KEYS */;
/*!40000 ALTER TABLE `userachievements` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `useritems`
--

DROP TABLE IF EXISTS `useritems`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `useritems` (
  `rowid` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `AccountID` int(11) NOT NULL,
  `IsEquipped` int(11) NOT NULL,
  `CharacterID` int(11) NOT NULL,
  `ItemID` int(11) NOT NULL,
  `ItemDuration` int(11) NOT NULL,
  `ItemNumber` int(11) NOT NULL,
  `ItemOrigin` int(11) NOT NULL,
  `acquisitionServerId` int(11) NOT NULL,
  `creationDate` bigint(20) unsigned NOT NULL,
  `durability` int(11) NOT NULL,
  `energy` int(11) NOT NULL,
  `isSealed` int(11) NOT NULL,
  `sealLevel` int(11) NOT NULL,
  `expEnhancement` int(11) NOT NULL,
  `mpEnhancement` int(11) NOT NULL,
  `IsCoupon` tinyint(1) DEFAULT NULL,
  `Stocks` int(11) DEFAULT 1,
  PRIMARY KEY (`rowid`),
  KEY `idx_useritems` (`AccountID`,`ItemNumber`)
) ENGINE=InnoDB AUTO_INCREMENT=14625625 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `useritems`
--

LOCK TABLES `useritems` WRITE;
/*!40000 ALTER TABLE `useritems` DISABLE KEYS */;
/*!40000 ALTER TABLE `useritems` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `users` (
  `AccountID` int(11) NOT NULL AUTO_INCREMENT,
  `AccountKey` bigint(20) DEFAULT NULL,
  `Username` varchar(17) NOT NULL,
  `Password` varchar(255) DEFAULT NULL,
  `Nickname` varchar(16) NOT NULL,
  `Grade` int(11) NOT NULL DEFAULT 1,
  `SuspendedUntil` varchar(25) DEFAULT '',
  `SuspensionReason` varchar(1500) DEFAULT NULL,
  `MutedUntil` text NOT NULL DEFAULT '0',
  `MuteReason` text DEFAULT NULL,
  `MutedBy` text NOT NULL DEFAULT '0',
  `Level` int(11) NOT NULL DEFAULT 10,
  `Experience` int(11) NOT NULL DEFAULT 35000,
  `Kills` int(11) NOT NULL DEFAULT 0,
  `Deaths` int(11) NOT NULL DEFAULT 0,
  `Assists` int(11) NOT NULL DEFAULT 0,
  `Wins` int(11) NOT NULL DEFAULT 0,
  `Loses` int(11) NOT NULL DEFAULT 0,
  `Draws` int(11) NOT NULL DEFAULT 0,
  `ClanID` int(11) DEFAULT 0,
  `ClanContribution` int(11) DEFAULT 0,
  `ClanKills` int(11) DEFAULT 0,
  `ClanDeaths` int(11) DEFAULT 0,
  `ClanAssists` int(11) DEFAULT 0,
  `ClanWins` int(11) DEFAULT 0,
  `ClanLoses` int(11) DEFAULT 0,
  `ClanDraws` int(11) DEFAULT 0,
  `MeleeKills` int(11) NOT NULL DEFAULT 0,
  `RifleKills` int(11) NOT NULL DEFAULT 0,
  `ShotgunKills` int(11) NOT NULL DEFAULT 0,
  `SniperKills` int(11) NOT NULL DEFAULT 0,
  `GatlingKills` int(11) NOT NULL DEFAULT 0,
  `BazookaKills` int(11) NOT NULL DEFAULT 0,
  `GrenadeKills` int(11) NOT NULL DEFAULT 0,
  `Headshots` int(11) NOT NULL DEFAULT 0,
  `HighestKillstreak` int(11) NOT NULL DEFAULT 0,
  `Playtime` int(11) NOT NULL DEFAULT 0,
  `ZombieKills` int(11) NOT NULL DEFAULT 0,
  `InfectedKills` int(11) NOT NULL DEFAULT 0,
  `MicroPoints` int(11) NOT NULL DEFAULT 0,
  `RockTotens` int(11) NOT NULL DEFAULT 100000000,
  `coins` int(11) unsigned DEFAULT NULL,
  `Battery` int(11) NOT NULL DEFAULT 5000,
  `MaxBattery` int(11) NOT NULL DEFAULT 5000,
  `MaxInventory` int(11) NOT NULL DEFAULT 200,
  `HasFinishedTutorial` int(11) NOT NULL DEFAULT 0,
  `SingleWaveAttempts` int(11) NOT NULL DEFAULT 100,
  `LastCharacterUsed` int(11) NOT NULL DEFAULT 0,
  `LuckyPoints` int(11) NOT NULL DEFAULT 0,
  `HighestSinglewaveScore` int(11) NOT NULL DEFAULT 0,
  `HighestSinglewaveStage` int(11) NOT NULL DEFAULT 0,
  `VipExperience` int(11) NOT NULL DEFAULT 0,
  `LatestWeeklyRewardDay` varchar(30) NOT NULL DEFAULT '0',
  `LatestMonthlyRewardDay` varchar(30) NOT NULL DEFAULT '0',
  `RoomCreationDisabledUntil` text DEFAULT NULL,
  `VotekickDisabledUntil` datetime DEFAULT NULL,
  `Secret` varchar(255) DEFAULT '',
  `Email` varchar(255) DEFAULT NULL,
  `LastLogged` datetime DEFAULT current_timestamp(),
  `LastIP` varchar(255) NOT NULL DEFAULT '',
  `LastIpSalt` varchar(128) DEFAULT NULL,
  `HWID` varchar(128) NOT NULL DEFAULT '',
  `HWIDSalt` varchar(32) DEFAULT NULL,
  `HWIDGraded` varchar(64) DEFAULT NULL,
  `HWIDGradedSalt` varchar(32) DEFAULT NULL,
  PRIMARY KEY (`AccountID`),
  UNIQUE KEY `uniqueNickname` (`Nickname`),
  UNIQUE KEY `uniqueUsername` (`Username`),
  CONSTRAINT `maxLevel` CHECK (`Level` <= 105),
  CONSTRAINT `maxBattery` CHECK (`Battery` <= 5000),
  CONSTRAINT `maxBatteryMax` CHECK (`MaxBattery` <= 5000),
  CONSTRAINT `maxInvNew` CHECK (`MaxInventory` <= 1000),
  CONSTRAINT `maxGrade` CHECK (`Grade` <= 8)
) ENGINE=InnoDB AUTO_INCREMENT=680165 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `users`
--

LOCK TABLES `users` WRITE;
/*!40000 ALTER TABLE `users` DISABLE KEYS */;
INSERT INTO `users` VALUES (1,0,'test','$2a$12$4Durul.zfqGsdqAOVCT0HuVK8wzZcA5lmKYwHF86tKEVkBaNpA1xm','test',2,'','','0',NULL,'0',10,35000,0,0,0,0,0,0,NULL,0,0,0,0,NULL,NULL,NULL,0,0,0,0,0,0,0,0,0,0,0,0,0,99991955,1,5000,5000,200,0,100,2,0,0,0,0,'2026-02-16','2026-02-16',NULL,NULL,'','','2026-02-16 14:04:11','','','','','','');
/*!40000 ALTER TABLE `users` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `weeklyrewards`
--

DROP TABLE IF EXISTS `weeklyrewards`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `weeklyrewards` (
  `ItemID` int(11) NOT NULL,
  `Date` varchar(30) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `weeklyrewards`
--

LOCK TABLES `weeklyrewards` WRITE;
/*!40000 ALTER TABLE `weeklyrewards` DISABLE KEYS */;
INSERT INTO `weeklyrewards` VALUES (4308102,'2026-02-16'),(4308102,'2026-02-16'),(5336542,'2026-02-16'),(5336544,'2026-02-16'),(5336542,'2026-02-16'),(5336544,'2026-02-16'),(4308101,'2026-02-16');
/*!40000 ALTER TABLE `weeklyrewards` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2026-02-16 18:03:58
