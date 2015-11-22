CREATE TABLE Users (
  ID_U INT UNSIGNED NOT NULL AUTO_INCREMENT,
  Name VARCHAR(64) NOT NULL,
  Email VARCHAR(255) NOT NULL,
  HPWD VARCHAR(255) NOT NULL,
  CreationTime TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  EmailValidated TINYINT(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (ID_U)
);

CREATE TABLE Tracks (
  ID_T INT UNSIGNED NOT NULL AUTO_INCREMENT,
  Filename VARCHAR(255) NOT NULL,
  Filesize INT UNSIGNED NOT NULL,
  ID_U INT UNSIGNED NOT NULL,
  Title VARCHAR(255),
  Artist VARCHAR(255),
  Mix VARCHAR(255),
  Year SMALLINT(4) UNSIGNED,
  Length SMALLINT UNSIGNED NOT NULL,
  Genre VARCHAR(64),
  TimeUploaded TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (ID_T),
  FOREIGN KEY (ID_U) REFERENCES Users(ID_U)
);

CREATE TABLE LoginAttempts (
  IP VARCHAR(50) NOT NULL,
  Attempts TINYINT UNSIGNED NOT NULL,
  LastLogin TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (IP)
);

CREATE TABLE EmailValidation (
  ID_U INT UNSIGNED NOT NULL,
  Hash VARCHAR(32) NOT NULL,
  PRIMARY KEY (ID_U),
  FOREIGN KEY (ID_U) REFERENCES Users(ID_U)
);

CREATE TABLE ErrorLogs (
  ID_E INT UNSIGNED NOT NULL AUTO_INCREMENT,
  DateLogged TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  ErrorNo INT UNSIGNED NOT NULL,
  ErrorString TEXT NOT NULL,
  ErrorFile VARCHAR(100) NOT NULL,
  ErrorLine INT UNSIGNED NOT NULL,
  PRIMARY KEY (ID_E)
);

CREATE USER 'custom1'@'localhost' IDENTIFIED BY 'obscure';
GRANT INSERT,UPDATE,SELECT,CREATE ON streamer.* to 'custom1'@'localhost';

// login
CREATE USER 'mMK6c4gYmJftrv86'@'localhost' IDENTIFIED BY 'pVvwviIGpofyIsTqfCk4UagCCaE8zra4';
GRANT INSERT,UPDATE,SELECT ON streamer.LoginAttempts to 'mMK6c4gYmJftrv86'@'localhost';
GRANT SELECT ON streamer.Users to 'mMK6c4gYmJftrv86'@'localhost';

// insert track
CREATE USER 'vNKZbYo7cFur03Y3'@'localhost' IDENTIFIED BY 'tZ6NA28fVu4QL0MIby0LBn8HRcsCZgch';
GRANT INSERT ON streamer.Tracks to 'vNKZbYo7cFur03Y3'@'localhost';

// create account
CREATE USER 'JjLRjogPstuoknPj'@'localhost' IDENTIFIED BY 'O2viZVurT8SVympgmMkAe61pq6RiKYSL';
GRANT INSERT,SELECT ON streamer.Users to 'JjLRjogPstuoknPj'@'localhost';

// insert email validation
CREATE USER 'MFqQA65FqmIat9CH'@'localhost' IDENTIFIED BY 'rBujpekIakbSFJLZz2YpjY598fBmWE8h';
GRANT INSERT ON streamer.EmailValidation to 'MFqQA65FqmIat9CH'@'localhost';

// get tracks
CREATE USER 'feVcRj2oGSdaxRWZ'@'localhost' IDENTIFIED BY '6Te1PwJji8Pg7tOXSSyJ0kU8jT4kg0hJ';
GRANT SELECT ON streamer.Tracks to 'feVcRj2oGSdaxRWZ'@'localhost';

// get user
CREATE USER 'cflrZ2bCcu6pyGh3'@'localhost' IDENTIFIED BY '0qbKxes7dFtq1AvCtPdsq3i42iUx6Q9H';
GRANT SELECT ON streamer.Users to 'cflrZ2bCcu6pyGh3'@'localhost';

// insert error
CREATE USER 'qm12XvA0NGGdjysr'@'localhost' IDENTIFIED BY '6sDDjsHQy65fIH9zOkAmgyb3J1okyFYN';
GRANT INSERT ON streamer.ErrorLogs to 'qm12XvA0NGGdjysr'@'localhost';
