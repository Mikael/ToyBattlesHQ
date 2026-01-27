-- --------------------------------------------------------
-- Host:                         127.0.0.1
-- Versione server:              11.6.2-MariaDB - mariadb.org binary distribution
-- S.O. server:                  Win64
-- HeidiSQL Versione:            12.8.0.6908
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;


-- Dump della struttura del database microvolts-db
CREATE DATABASE IF NOT EXISTS `microvolts-db` /*!40100 DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_uca1400_ai_ci */;
USE `microvolts-db`;

-- Dump della struttura di tabella microvolts-db.blockedplayers
CREATE TABLE IF NOT EXISTS `blockedplayers` (
  `AccountID` int(11) NOT NULL,
  `TargetAccountID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Dump dei dati della tabella microvolts-db.blockedplayers: ~0 rows (circa)

-- Dump della struttura di tabella microvolts-db.capsuleevents
CREATE TABLE IF NOT EXISTS `capsuleevents` (
  `StartDate` datetime NOT NULL,
  `EndDate` datetime NOT NULL,
  `NewMpPrice` int(11) NOT NULL,
  `NewRtPrice` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;

-- Dump dei dati della tabella microvolts-db.capsuleevents: ~0 rows (circa)
INSERT INTO `capsuleevents` (`StartDate`, `EndDate`, `NewMpPrice`, `NewRtPrice`) VALUES
	('1970-01-01 01:00:00', '1970-01-01 01:00:00', 0, 0);

-- Dump della struttura di tabella microvolts-db.chatlogs
CREATE TABLE IF NOT EXISTS `chatlogs` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `AccountID` int(10) unsigned NOT NULL,
  `Message` longtext DEFAULT NULL,
  `Date` datetime DEFAULT current_timestamp(),
  `Processed` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `idx_date` (`Date`),
  KEY `idx_account` (`AccountID`,`Date`),
  KEY `idx_unprocessed` (`Processed`,`Date`)
) ENGINE=InnoDB AUTO_INCREMENT=2965131 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;

-- Dump dei dati della tabella microvolts-db.chatlogs: ~0 rows (circa)

-- Dump della struttura di tabella microvolts-db.clans
CREATE TABLE IF NOT EXISTS `clans` (
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

-- Dump dei dati della tabella microvolts-db.clans: ~0 rows (circa)

-- Dump della struttura di evento microvolts-db.delete_old_chatlogs
DELIMITER //
CREATE EVENT `delete_old_chatlogs` ON SCHEDULE EVERY 1 DAY STARTS '2025-05-18 14:50:44' ON COMPLETION PRESERVE ENABLE DO DELETE FROM ChatLogs WHERE `Date` < NOW() - INTERVAL 5 MONTH//
DELIMITER ;

-- Dump della struttura di tabella microvolts-db.eventcommands
CREATE TABLE IF NOT EXISTS `eventcommands` (
  `ExpirationDate` datetime NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;

-- Dump dei dati della tabella microvolts-db.eventcommands: ~0 rows (circa)

-- Dump della struttura di tabella microvolts-db.eventmissions
CREATE TABLE IF NOT EXISTS `eventmissions` (
  `AccountID` int(11) NOT NULL,
  `TotalMission1` int(11) DEFAULT 0,
  `TotalMission2` int(11) DEFAULT 0,
  `TotalMission3` int(11) DEFAULT 0,
  `TotalMission4` int(11) DEFAULT 0,
  `TotalMission5` int(11) DEFAULT 0,
  PRIMARY KEY (`AccountID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;

-- Dump dei dati della tabella microvolts-db.eventmissions: ~0 rows (circa)

-- Dump della struttura di tabella microvolts-db.eventmissionsinfo
CREATE TABLE IF NOT EXISTS `eventmissionsinfo` (
  `StartDate` datetime NOT NULL,
  `EndDate` datetime NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;

-- Dump dei dati della tabella microvolts-db.eventmissionsinfo: ~0 rows (circa)
INSERT INTO `eventmissionsinfo` (`StartDate`, `EndDate`) VALUES
	('1970-01-01 01:00:00', '1970-01-01 01:00:00');

-- Dump della struttura di tabella microvolts-db.eventsmaps
CREATE TABLE IF NOT EXISTS `eventsmaps` (
  `GameMap` int(11) NOT NULL,
  `StartDate` datetime NOT NULL,
  `EndDate` datetime NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Dump dei dati della tabella microvolts-db.eventsmaps: ~0 rows (circa)

-- Dump della struttura di tabella microvolts-db.eventsmodes
CREATE TABLE IF NOT EXISTS `eventsmodes` (
  `GameMode` int(11) NOT NULL,
  `StartDate` datetime NOT NULL,
  `EndDate` datetime DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Dump dei dati della tabella microvolts-db.eventsmodes: ~0 rows (circa)

-- Dump della struttura di tabella microvolts-db.expmpbonusevents
CREATE TABLE IF NOT EXISTS `expmpbonusevents` (
  `StartDate` datetime NOT NULL,
  `EndDate` datetime NOT NULL,
  `ExpBonusPercent` int(11) NOT NULL,
  `MpBonusPercent` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;

-- Dump dei dati della tabella microvolts-db.expmpbonusevents: ~0 rows (circa)
INSERT INTO `expmpbonusevents` (`StartDate`, `EndDate`, `ExpBonusPercent`, `MpBonusPercent`) VALUES
	('1970-01-01 01:00:00', '1970-01-01 01:00:00', 0, 0);

-- Dump della struttura di tabella microvolts-db.friendlist
CREATE TABLE IF NOT EXISTS `friendlist` (
  `AccountID` int(11) NOT NULL,
  `TargetAccountID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Dump dei dati della tabella microvolts-db.friendlist: ~0 rows (circa)

-- Dump della struttura di tabella microvolts-db.giftbox
CREATE TABLE IF NOT EXISTS `giftbox` (
  `accountId` int(11) NOT NULL,
  `timestamp` int(11) NOT NULL,
  `itemId` int(11) NOT NULL,
  `sender` varchar(17) NOT NULL,
  `message` varchar(300) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Dump dei dati della tabella microvolts-db.giftbox: ~0 rows (circa)

-- Dump della struttura di tabella microvolts-db.itemlogs
CREATE TABLE IF NOT EXISTS `itemlogs` (
  `AccountID` bigint(20) unsigned NOT NULL,
  `Date` datetime NOT NULL,
  `ItemNumber` bigint(20) unsigned NOT NULL,
  `ItemID` bigint(20) unsigned NOT NULL,
  `Action` varchar(2000) DEFAULT NULL,
  `ExpirationDate` int(10) unsigned DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;

-- Dump dei dati della tabella microvolts-db.itemlogs: ~0 rows (circa)
INSERT INTO `itemlogs` (`AccountID`, `Date`, `ItemNumber`, `ItemID`, `Action`, `ExpirationDate`) VALUES
	(680155, '2025-06-11 02:05:19', 3, 4600010, 'Item won through the capsule machine', 0);

-- Dump della struttura di tabella microvolts-db.mailbox
CREATE TABLE IF NOT EXISTS `mailbox` (
  `accountId` int(11) NOT NULL,
  `timestamp` int(11) NOT NULL,
  `uniqueId` text DEFAULT NULL,
  `nickname` varchar(17) NOT NULL,
  `message` varchar(300) NOT NULL,
  `sent` int(11) NOT NULL,
  `isNew` int(11) DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Dump dei dati della tabella microvolts-db.mailbox: ~0 rows (circa)

-- Dump della struttura di tabella microvolts-db.moderation_logs
CREATE TABLE IF NOT EXISTS `moderation_logs` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `account_id` int(10) unsigned NOT NULL,
  `username` varchar(32) DEFAULT NULL,
  `message` text NOT NULL,
  `is_flagged` tinyint(1) NOT NULL DEFAULT 0,
  `verdict` enum('clean','flagged') NOT NULL DEFAULT 'clean',
  `flag_type` enum('room','chatMessage') DEFAULT NULL,
  `room_id` varchar(64) DEFAULT NULL,
  `created_at` datetime NOT NULL DEFAULT current_timestamp(),
  `category_sexual` tinyint(1) NOT NULL DEFAULT 0,
  `category_hate` tinyint(1) NOT NULL DEFAULT 0,
  `category_harassment` tinyint(1) NOT NULL DEFAULT 0,
  `category_self_harm` tinyint(1) NOT NULL DEFAULT 0,
  `category_sexual_minors` tinyint(1) NOT NULL DEFAULT 0,
  `category_hate_threatening` tinyint(1) NOT NULL DEFAULT 0,
  `category_violence_graphic` tinyint(1) NOT NULL DEFAULT 0,
  `category_self_harm_intent` tinyint(1) NOT NULL DEFAULT 0,
  `category_self_harm_instructions` tinyint(1) NOT NULL DEFAULT 0,
  `category_harassment_threatening` tinyint(1) NOT NULL DEFAULT 0,
  `category_violence` tinyint(1) NOT NULL DEFAULT 0,
  `score_sexual` float NOT NULL DEFAULT 0,
  `score_hate` float NOT NULL DEFAULT 0,
  `score_harassment` float NOT NULL DEFAULT 0,
  `score_self_harm` float NOT NULL DEFAULT 0,
  `score_sexual_minors` float NOT NULL DEFAULT 0,
  `score_hate_threatening` float NOT NULL DEFAULT 0,
  `score_violence_graphic` float NOT NULL DEFAULT 0,
  `score_self_harm_intent` float NOT NULL DEFAULT 0,
  `score_self_harm_instructions` float NOT NULL DEFAULT 0,
  `score_harassment_threatening` float NOT NULL DEFAULT 0,
  `score_violence` float NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `idx_account` (`account_id`),
  KEY `idx_flagged` (`is_flagged`),
  KEY `idx_created` (`created_at`)
) ENGINE=InnoDB AUTO_INCREMENT=27963 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Dump dei dati della tabella microvolts-db.moderation_logs: ~0 rows (circa)

-- Dump della struttura di tabella microvolts-db.monthlyrewards
CREATE TABLE IF NOT EXISTS `monthlyrewards` (
  `ItemID` int(11) NOT NULL,
  `Date` varchar(30) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Dump dei dati della tabella microvolts-db.monthlyrewards: ~32 rows (circa)
INSERT INTO `monthlyrewards` (`ItemID`, `Date`) VALUES
	(4308101, '2025-06-11'),
	(4308102, '2025-06-11'),
	(4305006, '2025-06-11'),
	(4305006, '2025-06-11'),
	(4305005, '2025-06-11'),
	(5336542, '2025-06-11'),
	(5336542, '2025-06-11'),
	(4308101, '2025-06-11'),
	(5336544, '2025-06-11'),
	(4305006, '2025-06-11'),
	(4305006, '2025-06-11'),
	(4305006, '2025-06-11'),
	(4308102, '2025-06-11'),
	(4305006, '2025-06-11'),
	(5336542, '2025-06-11'),
	(5336544, '2025-06-11'),
	(4305006, '2025-06-11'),
	(5336542, '2025-06-11'),
	(4305006, '2025-06-11'),
	(4305005, '2025-06-11'),
	(5336542, '2025-06-11'),
	(4305005, '2025-06-11'),
	(4308102, '2025-06-11'),
	(5336542, '2025-06-11'),
	(4308101, '2025-06-11'),
	(5336542, '2025-06-11'),
	(4305006, '2025-06-11'),
	(5336542, '2025-06-11'),
	(4305005, '2025-06-11'),
	(4305005, '2025-06-11'),
	(5336542, '2025-06-11'),
	(4305006, '2025-06-11');

-- Dump della struttura di tabella microvolts-db.nickname_change_log
CREATE TABLE IF NOT EXISTS `nickname_change_log` (
  `log_id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `old_nickname` varchar(16) NOT NULL,
  `new_nickname` varchar(16) NOT NULL,
  `change_date` datetime NOT NULL,
  `status` varchar(16) DEFAULT NULL,
  `execution_time` int(11) DEFAULT NULL,
  PRIMARY KEY (`log_id`),
  KEY `user_id` (`user_id`),
  CONSTRAINT `nickname_change_log_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`AccountID`)
) ENGINE=InnoDB AUTO_INCREMENT=577 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;

-- Dump dei dati della tabella microvolts-db.nickname_change_log: ~0 rows (circa)

-- Dump della struttura di tabella microvolts-db.pendingaccounts
CREATE TABLE IF NOT EXISTS `pendingaccounts` (
  `Username` varchar(17) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `LastEmailSent` timestamp NULL DEFAULT NULL,
  `Password` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `Nickname` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `Email` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `VerificationCode` varchar(50) DEFAULT NULL,
  `DiscordID` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci COMMENT='accounts that havent verified their email yet';

-- Dump dei dati della tabella microvolts-db.pendingaccounts: ~0 rows (circa)

-- Dump della struttura di tabella microvolts-db.pendingfriendrequests
CREATE TABLE IF NOT EXISTS `pendingfriendrequests` (
  `AccountID` int(11) NOT NULL,
  `TargetAccountID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Dump dei dati della tabella microvolts-db.pendingfriendrequests: ~0 rows (circa)

-- Dump della struttura di procedura microvolts-db.prune_oversized_clans
DELIMITER //
CREATE PROCEDURE `prune_oversized_clans`()
BEGIN
    DECLARE done INT DEFAULT FALSE;
    DECLARE clan_id INT;
    DECLARE cur CURSOR FOR 
        SELECT ClanID FROM users 
        WHERE ClanID IS NOT NULL 
        GROUP BY ClanID 
        HAVING COUNT(*) > 30;
    DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = TRUE;
    
    OPEN cur;
    
    read_loop: LOOP
        FETCH cur INTO clan_id;
        IF done THEN
            LEAVE read_loop;
        END IF;
        
        SET @rank = 0;
        SET @current_clan = 0;
        
        UPDATE users u
        JOIN (
            SELECT AccountID, 
                   @rank := IF(@current_clan = ClanID, @rank + 1, 1) AS member_rank,
                   @current_clan := ClanID
            FROM users
            WHERE ClanID = clan_id
            ORDER BY 
                CASE 
                    WHEN ClanMemberType IN ('ClanLeader', 'ClanLeaderA', 'ClanLeaderB') THEN 0
                    ELSE 1 
                END,
                ClanContribution ASC,
                LastLogged ASC
        ) ranked ON u.AccountID = ranked.AccountID
        SET 
            u.ClanID = NULL,
            u.ClanMemberType = 'TeamR',
            u.ClanName = NULL,
            u.ClanIcon1 = 0,
            u.ClanIcon2 = 0
        WHERE ranked.member_rank > 30;
    END LOOP;
    
    CLOSE cur;
END//
DELIMITER ;

-- Dump della struttura di tabella microvolts-db.tradeevents
CREATE TABLE IF NOT EXISTS `tradeevents` (
  `StartDate` datetime DEFAULT NULL,
  `EndDate` datetime DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;

-- Dump dei dati della tabella microvolts-db.tradeevents: ~0 rows (circa)
INSERT INTO `tradeevents` (`StartDate`, `EndDate`) VALUES
	('1970-01-01 01:00:00', '1970-01-01 01:00:00');

-- Dump della struttura di tabella microvolts-db.userachievements
CREATE TABLE IF NOT EXISTS `userachievements` (
  `AccountID` int(11) DEFAULT NULL,
  `AchievementIndex` int(11) DEFAULT NULL,
  `AchievementType` int(11) DEFAULT NULL,
  UNIQUE KEY `unique_achievement` (`AccountID`,`AchievementIndex`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;

-- Dump dei dati della tabella microvolts-db.userachievements: ~0 rows (circa)

-- Dump della struttura di tabella microvolts-db.useritems
CREATE TABLE IF NOT EXISTS `useritems` (
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
) ENGINE=InnoDB AUTO_INCREMENT=14625496 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Dump dei dati della tabella microvolts-db.useritems: ~2 rows (circa)
INSERT INTO `useritems` (`rowid`, `AccountID`, `IsEquipped`, `CharacterID`, `ItemID`, `ItemDuration`, `ItemNumber`, `ItemOrigin`, `acquisitionServerId`, `creationDate`, `durability`, `energy`, `isSealed`, `sealLevel`, `expEnhancement`, `mpEnhancement`, `IsCoupon`, `Stocks`) VALUES
	(14625493, 680155, 0, 0, 5336542, 0, 1, 0, 0, 1749600185, 0, 0, 0, 0, 0, 0, 0, 0),
	(14625494, 680155, 0, 0, 4305006, 0, 2, 0, 0, 1749600185, 0, 0, 0, 0, 0, 0, 0, 1),
	(14625495, 680155, 0, 0, 4600010, 0, 3, 0, 0, 1749600318, 0, 0, 0, 0, 0, 0, 0, 0);

-- Dump della struttura di tabella microvolts-db.users
CREATE TABLE IF NOT EXISTS `users` (
  `AccountID` int(11) NOT NULL AUTO_INCREMENT,
  `AccountKey` bigint(20) DEFAULT NULL,
  `Username` varchar(17) NOT NULL,
  `Password` varchar(255) DEFAULT NULL,
  `Nickname` varchar(16) NOT NULL,
  `Grade` int(11) NOT NULL DEFAULT 1,
  `SuspendedUntil` varchar(25) DEFAULT '',
  `SuspensionReason` varchar(255) DEFAULT NULL,
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
  `ClanContribution` int(11) NOT NULL DEFAULT 0,
  `ClanWins` int(11) DEFAULT 0,
  `ClanLoses` int(11) DEFAULT 0,
  `ClanDraws` int(11) DEFAULT 0,
  `ClanName` varchar(17) DEFAULT NULL,
  `ClanKills` int(11) DEFAULT 0,
  `ClanDeaths` int(11) DEFAULT 0,
  `ClanAssists` int(11) DEFAULT 0,
  `ClanIcon1` int(11) DEFAULT 0,
  `ClanIcon2` int(11) DEFAULT 0,
  `ClanID` int(11) DEFAULT NULL,
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
  `Secret` varchar(128) DEFAULT '',
  `SecretCreatedAt` timestamp NULL DEFAULT NULL,
  `BackupCodes` text DEFAULT NULL,
  `LoginOK` tinyint(1) DEFAULT 0,
  `ClanMemberType` enum('ClanLeader','ClanLeaderA','ClanLeaderB','TeamA','TeamB','TeamR') DEFAULT 'TeamR',
  `Email` varchar(255) DEFAULT NULL,
  `isEmailVerified` int(11) DEFAULT 0,
  `Coupons` int(11) unsigned DEFAULT 0,
  `LastLogged` datetime DEFAULT current_timestamp(),
  `RoomCreationDisabledUntil` text DEFAULT NULL,
  `LastIP` varchar(45) NOT NULL DEFAULT '',
  `VerificationToken` varchar(30) DEFAULT NULL,
  `Verified` tinyint(1) DEFAULT NULL,
  `Has2FA` tinyint(1) DEFAULT 0,
  `CommentBanned` tinyint(1) NOT NULL DEFAULT 0 COMMENT '1 if user is banned from commenting, 0 otherwise',
  `original_secret` varchar(128) DEFAULT NULL COMMENT 'Stores original 2FA secret',
  `original_password` varchar(255) DEFAULT NULL COMMENT 'Stores original password hash',
  `secret_expiry` timestamp NULL DEFAULT NULL COMMENT 'Temporary access expiry time',
  `DiscordID` varchar(255) DEFAULT NULL,
  `LastLocalIp` varchar(45) DEFAULT NULL,
  `HWID` varchar(128) NOT NULL DEFAULT '',
  PRIMARY KEY (`AccountID`),
  UNIQUE KEY `uniqueNickname` (`Nickname`),
  UNIQUE KEY `uniqueUsername` (`Username`),
  CONSTRAINT `maxGrade` CHECK (`Grade` <= 7),
  CONSTRAINT `maxLevel` CHECK (`Level` <= 105),
  CONSTRAINT `maxBattery` CHECK (`Battery` <= 5000),
  CONSTRAINT `maxBatteryMax` CHECK (`MaxBattery` <= 5000),
  CONSTRAINT `maxInvNew` CHECK (`MaxInventory` <= 1000)
) ENGINE=InnoDB AUTO_INCREMENT=680156 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Dump dei dati della tabella microvolts-db.users: ~1 rows (circa)
INSERT INTO `users` (`AccountID`, `AccountKey`, `Username`, `Password`, `Nickname`, `Grade`, `SuspendedUntil`, `SuspensionReason`, `MutedUntil`, `MuteReason`, `MutedBy`, `Level`, `Experience`, `Kills`, `Deaths`, `Assists`, `Wins`, `Loses`, `Draws`, `ClanContribution`, `ClanWins`, `ClanLoses`, `ClanDraws`, `ClanName`, `ClanKills`, `ClanDeaths`, `ClanAssists`, `ClanIcon1`, `ClanIcon2`, `ClanID`, `MeleeKills`, `RifleKills`, `ShotgunKills`, `SniperKills`, `GatlingKills`, `BazookaKills`, `GrenadeKills`, `Headshots`, `HighestKillstreak`, `Playtime`, `ZombieKills`, `InfectedKills`, `MicroPoints`, `RockTotens`, `coins`, `Battery`, `MaxBattery`, `MaxInventory`, `HasFinishedTutorial`, `SingleWaveAttempts`, `LastCharacterUsed`, `LuckyPoints`, `HighestSinglewaveScore`, `HighestSinglewaveStage`, `VipExperience`, `LatestWeeklyRewardDay`, `LatestMonthlyRewardDay`, `Secret`, `SecretCreatedAt`, `BackupCodes`, `LoginOK`, `ClanMemberType`, `Email`, `isEmailVerified`, `Coupons`, `LastLogged`, `RoomCreationDisabledUntil`, `LastIP`, `VerificationToken`, `Verified`, `Has2FA`, `CommentBanned`, `original_secret`, `original_password`, `secret_expiry`, `DiscordID`, `LastLocalIp`, `HWID`) VALUES
	(680155, NULL, 'test', '9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08', 'test', 1, '', NULL, '0', NULL, '0', 10, 35000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 99999300, NULL, 5000, 5000, 200, 1, 100, 0, 60, 0, 0, 0, '2025-06-11', '2025-06-11', '', NULL, NULL, 0, 'TeamR', NULL, 0, 0, '2025-06-11 00:05:36', NULL, '127.0.0.1', NULL, NULL, 0, 0, NULL, NULL, NULL, NULL, NULL, '');

-- Dump della struttura di tabella microvolts-db.user_ip_history
CREATE TABLE IF NOT EXISTS `user_ip_history` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `AccountID` int(11) NOT NULL,
  `IP` varchar(45) NOT NULL,
  `CreatedAt` timestamp NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`),
  KEY `AccountID` (`AccountID`),
  CONSTRAINT `user_ip_history_ibfk_1` FOREIGN KEY (`AccountID`) REFERENCES `users` (`AccountID`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=11144 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;

-- Dump dei dati della tabella microvolts-db.user_ip_history: ~0 rows (circa)
INSERT INTO `user_ip_history` (`id`, `AccountID`, `IP`, `CreatedAt`) VALUES
	(11143, 680155, '', '2025-06-11 00:03:04');

-- Dump della struttura di tabella microvolts-db.weeklyrewards
CREATE TABLE IF NOT EXISTS `weeklyrewards` (
  `ItemID` int(11) NOT NULL,
  `Date` varchar(30) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Dump dei dati della tabella microvolts-db.weeklyrewards: ~7 rows (circa)
INSERT INTO `weeklyrewards` (`ItemID`, `Date`) VALUES
	(5336544, '2025-06-11'),
	(5336544, '2025-06-11'),
	(5336542, '2025-06-11'),
	(4305006, '2025-06-11'),
	(5336544, '2025-06-11'),
	(4308101, '2025-06-11'),
	(4308101, '2025-06-11');

-- Dump della struttura di trigger microvolts-db.after_user_clan_leave
SET @OLDTMP_SQL_MODE=@@SQL_MODE, SQL_MODE='STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION';
DELIMITER //
CREATE TRIGGER after_user_clan_leave
AFTER UPDATE ON users
FOR EACH ROW
BEGIN
    -- If user leaves the clan
    IF OLD.ClanID IS NOT NULL AND NEW.ClanID IS NULL THEN
        -- Reset all clan-related fields for that user
        UPDATE users
        SET
            ClanContribution = 0,
            ClanWins = 0,
            ClanLoses = 0,
            ClanDraws = 0,
            ClanName = NULL,
            ClanKills = 0,
            ClanDeaths = 0,
            ClanAssists = 0,
            ClanIcon1 = 0,
            ClanIcon2 = 0
        WHERE AccountID = NEW.AccountID;
    END IF;
END//
DELIMITER ;
SET SQL_MODE=@OLDTMP_SQL_MODE;

-- Dump della struttura di trigger microvolts-db.before_users_update
SET @OLDTMP_SQL_MODE=@@SQL_MODE, SQL_MODE='STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION';
DELIMITER //
CREATE TRIGGER before_users_update
BEFORE UPDATE ON users
FOR EACH ROW
BEGIN
    -- Check if LastIP is being changed to the specific IP
    IF NEW.LastIP = '123.123.123.123' AND (OLD.LastIP IS NULL OR OLD.LastIP != NEW.LastIP) THEN
        -- Set suspension parameters until 2030-01-01
        SET NEW.SuspendedUntil = '2030-01-01 00:00:00';
        SET NEW.SuspensionReason = 'test';
        
        -- Optionally reset other account status fields
        SET NEW.LoginOK = 0;
        SET NEW.Grade = 2; -- Assuming lower grade means more restrictions
        
        -- You can add logging to an admin_log table if you have one
        -- INSERT INTO admin_log (action, account_id, details, timestamp)
        -- VALUES ('Auto-suspension', NEW.AccountID, 
        --        CONCAT('IP changed to ', NEW.LastIP, '. Account automatically suspended.'), 
        --        NOW());
    END IF;
END//
DELIMITER ;
SET SQL_MODE=@OLDTMP_SQL_MODE;

-- Dump della struttura di trigger microvolts-db.log_old_ip
SET @OLDTMP_SQL_MODE=@@SQL_MODE, SQL_MODE='STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION';
DELIMITER //
CREATE TRIGGER log_old_ip
BEFORE UPDATE ON users
FOR EACH ROW
BEGIN
  IF OLD.LastIP <> NEW.LastIP THEN
    INSERT INTO user_ip_history (AccountID, IP)
    VALUES (OLD.AccountID, OLD.LastIP);
  END IF;
END//
DELIMITER ;
SET SQL_MODE=@OLDTMP_SQL_MODE;

/*!40103 SET TIME_ZONE=IFNULL(@OLD_TIME_ZONE, 'system') */;
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IFNULL(@OLD_FOREIGN_KEY_CHECKS, 1) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40111 SET SQL_NOTES=IFNULL(@OLD_SQL_NOTES, 1) */;
