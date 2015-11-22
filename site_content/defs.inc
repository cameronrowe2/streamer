<?php

// DB CONNECTIONS

// login
define("LOGIN_USR", "mMK6c4gYmJftrv86");
define("LOGIN_PWD", "pVvwviIGpofyIsTqfCk4UagCCaE8zra4");

// insert track
define("INSERT_TRACK_USR", "vNKZbYo7cFur03Y3");
define("INSERT_TRACK_PWD", "tZ6NA28fVu4QL0MIby0LBn8HRcsCZgch");

// create account
define("CREATE_ACCOUNT_USR", "JjLRjogPstuoknPj");
define("CREATE_ACCOUNT_PWD", "O2viZVurT8SVympgmMkAe61pq6RiKYSL");

// insert emailv
define("INSERT_EMAILV_USR", "MFqQA65FqmIat9CH");
define("INSERT_EMAILV_PWD", "rBujpekIakbSFJLZz2YpjY598fBmWE8h");

// get tracks
define("GET_TRACKS_USR", "feVcRj2oGSdaxRWZ");
define("GET_TRACKS_PWD", "6Te1PwJji8Pg7tOXSSyJ0kU8jT4kg0hJ");

// get user
define("GET_USER_USR", "cflrZ2bCcu6pyGh3");
define("GET_USER_PWD", "0qbKxes7dFtq1AvCtPdsq3i42iUx6Q9H");

// log err
define("LOG_ERR_USR", "qm12XvA0NGGdjysr");
define("LOG_ERR_PWD", "6sDDjsHQy65fIH9zOkAmgyb3J1okyFYN");

/*
  Errors
  - trigger_error
  - error_log
*/

function setup_err_funcs() {
  register_shutdown_function("fatal_handler");
  set_error_handler("error_handler");
}

function fatal_handler() {
  $error = error_get_last();
  if ( $error["type"] == E_ERROR )
      log_error( $error["type"], $error["message"], $error["file"], $error["line"] );
}

function error_handler($errno, $errstr, $errfile, $errline) {
  log_error($errno, $errstr, $errfile, $errline);
}


// insert error
function log_error($errno, $errstr, $errfile, $errline) {

  error_log('ErrorNo: '.$errno.'\t\tErrorString: '.$errstr.'\t\tErrorFile: '.$errfile.'\t\tErrorLine: '.$errline);

  $conn = new mysqli('localhost', LOG_ERR_USR, LOG_ERR_PWD, 'streamer');

  // Check connection
  if ($conn->connect_error) {
      $to      = 'cameronroweau@gmail.com'; // Send email to our user
      $subject = 'STREAMER MAJOR ERROR! Cannot log error'; // Give the email a subject
      $message = '

      Errors cannot be logged! The system cannot connect to the database to log an error.
      Fix this immediately.

      ';

      $headers = 'From:mail@streamer.online' . "\r\n"; // Set from headers
      if( !mail($to, $subject, $message, $headers) ) {
          error_log('MAJOR: Cannot send mail due to failed db connection for error log');
      }

      trigger_error('Func: log_error. MYSQL Connection failed: '.$conn->connect_error, E_USER_ERROR);
      die();
  }

  $stmt = $conn->prepare("INSERT INTO ErrorLogs (ErrorNo, ErrorString, ErrorFile, ErrorLine) VALUES (?, ?, ?, ?)");
  $stmt->bind_param("issi", $errno, $errstr, $errfile, $errline);
  $stmt->execute();
  $stmt->close();

  $conn->close();
}

// returns true if email exists, and if there is onlt one row for an email address
function email_exists($email) {
  // Create connection
  $conn = new mysqli('localhost', GET_USER_USR, GET_USER_PWD, 'streamer');

  // Check connection
  if ($conn->connect_error) {
      trigger_error('Func: email_exists. MYSQL Connection failed: '.$conn->connect_error, E_USER_ERROR);
      die();
  }

  $stmt = $conn->prepare("SELECT Email FROM Users WHERE Email=?");
  $stmt->bind_param("s", $email);
  $stmt->execute();
  $result = $stmt->get_result();
  $stmt->close();

  $conn->close();

  if($result->num_rows === 1) {
    return true;
  }

  if($result->num_rows > 1) {
    trigger_error('Func: email_exists. Number of existing emails\''.$email.'\' should be 1, but is: '.$result->num_rows, E_USER_ERROR);
    die();
  }

  return false;
}

// get login attempts (2 or less) - will create ip record if one doesnt exist
function get_login_attempts($conn, $ip) {
  $attempts = 0;

  $stmt = $conn->prepare("SELECT Attempts, LastLogin FROM LoginAttempts WHERE IP=?");
  $stmt->bind_param("s", $ip);
  $stmt->execute();
  $result = $stmt->get_result();
  $stmt->close();

  // check num results
  switch($result->num_rows) {
    case 0:
      if ( !($stmt = $conn->prepare("INSERT INTO LoginAttempts (IP) VALUES (?)") ) ) {
        trigger_error('Func: get_login_attempts. MYSQL - INSERT LoginAttempts: prepare error', E_USER_ERROR);
        die();
      }
      if ( !($stmt->bind_param("s", $ip) ) ) {
        trigger_error('Func: get_login_attempts. MYSQL - INSERT LoginAttempts: bind_param error', E_USER_ERROR);
        die();
      }
      $stmt->execute();
      $stmt->close();

      break;

    case 1:
      $row = $result->fetch_assoc();
      if( (strtotime("now") - strtotime($row['LastLogin']) ) > 600) { // concern
        set_login_attempts($conn, $ip, 0);
      } else {
        // return login attempts
        $attempts = $row['Attempts'];
      }
      break;

    default: // other >1
      trigger_error('Func: get_login_attempts. LoginAttempts num_rows > 1: '.$result->num_rows, E_USER_WARNING);
      $attempts = 100;
      break;
  }

  return $attempts;
}

// sets login attempts for an ip address
function set_login_attempts($conn, $ip, $attempts) {
  if ( !($stmt = $conn->prepare("UPDATE LoginAttempts SET Attempts=? WHERE IP=?") ) ) {
    trigger_error('Func: set_login_attempts. MYSQL - UPDATE LoginAttempts: prepare error', E_USER_ERROR);
    die();
  }
  if ( !($stmt->bind_param("is", $attempts, $ip) ) ) {
    trigger_error('Func: set_login_attempts. MYSQL - UPDATE LoginAttempts: bind_param error', E_USER_ERROR);
    die();
  }
  $stmt->execute();
  $stmt->close();
}

// return false if no match
function login_match($conn, $email, $pwd) {
  // get data
  $stmt = $conn->prepare("SELECT ID_U, HPWD FROM Users WHERE Email=?");
  $stmt->bind_param("s", $email);
  $stmt->execute();
  $result = $stmt->get_result();
  $stmt->close();

  if($result->num_rows === 1) {
    $row = $result->fetch_assoc();

    if( password_verify($pwd, $row['HPWD']) ) {
      return $row['ID_U'];
    }
    return false;
  }

  if($result->num_rows > 1) {
    trigger_error('Func: login_match. Number of accounts with email: '.$email.' is greater than 1: '.$result->num_rows, E_USER_ERROR);
    die();
  }

  return false;
}

// login
function login($ip, $email, $pwd) {
  // Create connection
  $conn = new mysqli('localhost', LOGIN_USR, LOGIN_PWD, 'streamer'); // db data
  if ($conn->connect_error) {
      trigger_error('Func: login. MYSQL Connection failed: '.$conn->connect_error, E_USER_ERROR);
      die();
  }

  // get login attempts - will create ip record if one doesnt exist
  if( ( $login_attempts = get_login_attempts($conn, $ip) ) < 7 )
  {
    // sanitisation - check values against db
    if( login_valid($email, $pwd) && is_int($user_id = login_match($conn, $email, $pwd) ) ) {
      // store login details as session vars
      $_SESSION['loggedIn'] = true;
      $_SESSION['user_id'] = $user_id;
      $_SESSION['user_email'] = $email;

    } else {
      set_login_attempts($conn, $ip, ++$login_attempts);
      echo("Incorrect Login Details");
    }

  } else {
    echo("Incorrect login details too many times. Wait 10 mins before logging in again");
  }

  $conn->close();
}

function login_valid($email, $pwd) {
  // EMAIL
  if( !isset($email) || empty($email) ) {
    return false;
  }
  if( strlen($email) > 255 ) {
    return false;
  }
  // email is valid (xxx@xxx.xxx)(255 limit)
  if( !preg_match('/^([\w-]+(?:\.[\w-]+)*)@((?:[\w-]+\.)*\w[\w-]{0,66})\.([a-z]{2,6}(?:\.[a-z]{2})?)$/', $email) ) {
    return false;
  }
  // invalid - Invalid email address
    //@
    //.
    //uname
    //domain
    //tld

  // PWD

  // empty
  if( !isset($pwd) || empty($pwd) ) {
    return false;
  }
  // length - Please enter a password between 8 and 64 characters
  // invalid - Your email must contain at least 1 lowercase, uppercase, and number/special character
  if( !isset($pwd) || empty($pwd) || //strlen($pwd_1) < 6 || strlen($pwd_1) > 64 ) {
      !preg_match('/(?=^.{8,64}$)((?=.*\d)|(?=.*\W+))(?![.\n])(?=.*[A-Z])(?=.*[a-z]).*$/', $pwd) ) {
      return false;
  }

  return true;
}

// get_upload_error_message
function get_upload_error_message($code) {
    $message = 'Upload File Error '.$code.': ';

    switch ($code) {
        case UPLOAD_ERR_INI_SIZE:
            $message .= "The uploaded file exceeds the upload_max_filesize directive in php.ini";
            break;
        case UPLOAD_ERR_FORM_SIZE:
            $message .= "The uploaded file exceeds the MAX_FILE_SIZE directive that was specified in the HTML form";
            break;
        case UPLOAD_ERR_PARTIAL:
            $message .= "The uploaded file was only partially uploaded";
            break;
        case UPLOAD_ERR_NO_FILE:
            $message .= "No file was uploaded";
            break;
        case UPLOAD_ERR_NO_TMP_DIR:
            $message .= "Missing a temporary folder";
            break;
        case UPLOAD_ERR_CANT_WRITE:
            $message .= "Failed to write file to disk";
            break;
        case UPLOAD_ERR_EXTENSION:
            $message .= "File upload stopped by extension";
            break;

        default:
            $message .= "Unknown upload error";
            break;
    }
    return $message;
}

// ensures that the given filename does not already exist in the database, if so, the filename is changed
function validate_track_filename($user_id, $filename){
  $validated = $filename;

  // Create connection
  $conn = new mysqli('localhost', GET_TRACKS_USR, GET_TRACKS_PWD, 'streamer'); // db data
  if ($conn->connect_error) {
      trigger_error('Func: validate_track_filename. MYSQL Connection failed: '.$conn->connect_error, E_USER_ERROR);
      die();
  }

  $stmt = $conn->prepare("SELECT Filename FROM Tracks WHERE ID_U=? and Filename=?");
  $stmt->bind_param("is", $user_id, $filename);
  $stmt->execute();
  $result = $stmt->get_result();
  $stmt->close();

  if($result->num_rows > 0) {
      // modify name
      for($i = 97; $i < 123; $i++) {
        $new_filename = substr_replace($filename,chr($i),strrpos($filename, '.')-1,1);

        $stmt = $conn->prepare("SELECT Filename FROM Tracks WHERE ID_U=? and Filename=?");
        $stmt->bind_param("is", $user_id, $new_filename);
        $stmt->execute();
        $result_n = $stmt->get_result();
        $stmt->close();

        if($result_n->num_rows === 0) {
          $validated = $new_filename;
          break;
        }

        if($i === 122) {
          $validated = false;
        }
      }
  }

  $conn->close();

  return $validated;
}

// gets the id3 tags for an mp3 file
function get_ID3_tags($file) {

    $id3tags = array();

    require_once('getid3/getid3.php');

    // Initialize getID3 engine
    $getID3 = new getID3;

    // Analyze file and store returned data in $ThisFileInfo
    $fileInfo = $getID3->analyze($file);

    // Optional: copies data from all subarrays of [tags] into [comments] so
    // metadata is all available in one location for all tag formats
    // metainformation is always available under [tags] even if this is not called
    getid3_lib::CopyTagsToComments($fileInfo);

    // Output desired information in whatever format you want
    // Note: all entries in [comments] or [tags] are arrays of strings
    // See structure.txt for information on what information is available where
    // or check out the output of /demos/demo.browse.php for a particular file
    // to see the full detail of what information is returned where in the array
    // Note: all array keys may not always exist, you may want to check with isset()
    // or empty() before deciding what to output
    if( !isset($fileInfo) ){
        trigger_error('Func: get_ID3_tags. $fileInfo not set after analysing file: '.$file.'. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
        die();
    }
    if( empty($fileInfo) ){
        trigger_error('Func: get_ID3_tags. $fileInfo empty after analysing file: '.$file.'. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
        die();
    }

    // title
    $id3tags['Title'] = ( ( !isset($fileInfo['comments']['title'][0]) ) ? NULL : $fileInfo['comments']['title'][0]);

    // artist
    $id3tags['Artist'] = ( ( !isset($fileInfo['comments']['artist'][0]) ) ? NULL : $fileInfo['comments']['artist'][0]);

    // Mix
    $id3tags['Mix'] = NULL;

    // year
    $id3tags['Year'] = ( ( !isset($fileInfo['comments']['year'][0]) ) ? NULL : intval($fileInfo['comments']['year'][0]) );

    // length - seconds
    $id3tags['Length'] = ( ( !isset($fileInfo['playtime_seconds']) ) ? NULL : intval($fileInfo['playtime_seconds']) );

    // genre
    $id3tags['Genre'] = ( ( !isset($fileInfo['comments']['genre'][0]) ) ? NULL : $fileInfo['comments']['genre'][0]);

    return $id3tags;
}

// checks if a var is set and not empty (nempty)
function set_nempty($var){
  if( isset($var) && !empty($var) ) {
    return false;
  } else {
    return true;
  }
}

// saves a track in the db for a user and returns the track id
function save_track_db($user_id, $filename, $file, $id3tags) {

  // Create connection
  $conn = new mysqli('localhost', INSERT_TRACK_USR, INSERT_TRACK_PWD, 'streamer'); // db data
  if ($conn->connect_error) {
      trigger_error('Func: save_track_db. MYSQL Connection Failed: '.$conn->connect_error, E_USER_ERROR);
      die();
  }

  if( !($stmt = $conn->prepare("INSERT INTO Tracks (Filename, Filesize, ID_U, Title, Artist, Year, Length, Genre) VALUES (?, ?, ?, ?, ?, ?, ?, ?)") ) ) {
    trigger_error('Func: save_track_db. MYSQL - INSERT Track: prepare error', E_USER_ERROR);
    die();
  }

  $tmp = array(
    '$filename' => $filename,
    '$filesize' => $file['size'],
    '$user_id' => $user_id,
    'Title' => $id3tags['Title'],
    'Artist' => $id3tags['Artist'],
    'Year' => $id3tags['Year'],
    'Length' => $id3tags['Length'],
    'Genre' => $id3tags['Genre']
  );

  if( !($stmt->bind_param("siissiis", $filename, $file['size'], $user_id, $id3tags['Title'], $id3tags['Artist'], $id3tags['Year'], $id3tags['Length'], $id3tags['Genre']) ) ) {
    trigger_error('Func: save_track_db. MYSQL - INSERT Track: bind_param error', E_USER_ERROR);
    die();
  }
  if(!$stmt->execute()) {
    trigger_error('Func: save_track_db. MYSQL - INSERT Track: execute error', E_USER_ERROR);
    die();
  }

  $last_id = $stmt->insert_id;
  if($last_id === 0) {
    trigger_error('Func: save_track_db. Logic - INSERT Track: $last_id === 0 of last inserted track. $last_id should exist.', E_USER_ERROR);
    die();
  }

  $stmt->close();
  $conn->close();

  return $last_id;
}

// checks if a user is a robot
function client_robot($key) {
  $url = 'https://www.google.com/recaptcha/api/siteverify';

  $data = array(
    'secret' => '6Lft1wsTAAAAACY1ZuIk4k4KuYIQ1U8DKUWNsW_b', // secret
    'response' => $key,
    'remoteip' => $_SERVER['REMOTE_ADDR']
  );

  // use key 'http' even if you send the request to https://...
  $options = array(
      'http' => array(
          'header'  => "Content-type: application/x-www-form-urlencoded\r\n",
          'method'  => 'POST',
          'content' => http_build_query($data),
      ),
      "ssl"=>array(
        "verify_peer"     =>  false,
        "verify_peer_name"=>  false
      )
  );
  $context  = stream_context_create($options);

  $result = file_get_contents($url, false, $context);
  if($result === false) {
    trigger_error('Func: client_robot. Captcha - file_get_contents fail for retrieved captch result.');
    return true;
  }

  $res_arr = json_decode($result, true);
  if($res_arr["success"] === true) {
    return false;
  }

  // can't log robot because I'll overload db
  return true;
}


// UNUSED - servs a file as resumable - uses HTTP_RANGE requests
/*
function serve_file_resumable($file, $contenttype = 'application/octet-stream') {

    // Avoid sending unexpected errors to the client - we should be serving a file,
    // we don't want to corrupt the data we send
    @error_reporting(0);

    // Make sure the files exists, otherwise we are wasting our time
    if (!file_exists($file)) {
      header("HTTP/1.1 404 Not Found");
      exit;
    }

    // Get the 'Range' header if one was sent
    if (isset($_SERVER['HTTP_RANGE'])) $range = $_SERVER['HTTP_RANGE']; // IIS/Some Apache versions
    else if ($apache = apache_request_headers()) { // Try Apache again
      $headers = array();
      foreach ($apache as $header => $val) $headers[strtolower($header)] = $val;
      if (isset($headers['range'])) $range = $headers['range'];
      else $range = FALSE; // We can't get the header/there isn't one set
    } else $range = FALSE; // We can't get the header/there isn't one set

    // Get the data range requested (if any)
    $filesize = filesize($file);
    if ($range) {
      $partial = true;
      list($param,$range) = explode('=',$range);
      if (strtolower(trim($param)) != 'bytes') { // Bad request - range unit is not 'bytes'
        header("HTTP/1.1 400 Invalid Request");
        exit;
      }
      $range = explode(',',$range);
      $range = explode('-',$range[0]); // We only deal with the first requested range
      if (count($range) != 2) { // Bad request - 'bytes' parameter is not valid
        header("HTTP/1.1 400 Invalid Request");
        exit;
      }
      if ($range[0] === '') { // First number missing, return last $range[1] bytes
        $end = $filesize - 1;
        $start = $end - intval($range[0]);
      } else if ($range[1] === '') { // Second number missing, return from byte $range[0] to end
        $start = intval($range[0]);
        $end = $filesize - 1;
      } else { // Both numbers present, return specific range
        $start = intval($range[0]);
        $end = intval($range[1]);
        if ($end >= $filesize || (!$start && (!$end || $end == ($filesize - 1)))) $partial = false; // Invalid range/whole file specified, return whole file
      }
      $length = $end - $start + 1;
    } else $partial = false; // No range requested

    // Send standard headers
    header("Content-Type: $contenttype");
    header("Content-Length: $filesize");
    header('Content-Disposition: attachment; filename="'.basename($file).'"');
    header('Accept-Ranges: bytes');

    // if requested, send extra headers and part of file...
    if ($partial) {
      header('HTTP/1.1 206 Partial Content');
      header("Content-Range: bytes $start-$end/$filesize");
      if (!$fp = fopen($file, 'r')) { // Error out if we can't read the file
        header("HTTP/1.1 500 Internal Server Error");
        exit;
      }
      if ($start) fseek($fp,$start);
      while ($length) { // Read in blocks of 8KB so we don't chew up memory on the server
        $read = ($length > 8192) ? 8192 : $length;
        $length -= $read;
        print(fread($fp,$read));
      }
      fclose($fp);
    } else readfile($file); // ...otherwise just send the whole file

    // Exit here to avoid accidentally sending extra content on the end of the file
    exit;

}
*/

// create account
function create_account_db($name, $email, $pwd_hash) {
  // Create connection
  $conn = new mysqli('localhost', CREATE_ACCOUNT_USR, CREATE_ACCOUNT_PWD, 'streamer'); // db data
  if ($conn->connect_error) {
      trigger_error('Func: create_account_db. MYSQL Connection failed: '.$conn->connect_error, E_USER_WARNING);
      die('Sorry, your account could not be created');
  }

  // prepare and bind
  if ( !($stmt = $conn->prepare("INSERT INTO Users (Name, Email, HPWD) VALUES (?, ?, ?)") ) ) {
    trigger_error('Func: create_account_db. MYSQL - INSERT User: prepare error', E_USER_WARNING);
    die('Sorry, your account could not be created');
  }
  if ( !($stmt->bind_param("sss", $name, $email, $pwd_hash) ) ) {
    trigger_error('Func: create_account_db. MYSQL - INSERT User: bind_param error', E_USER_WARNING);
    die('Sorry, your account could not be created');
  }

  $stmt->execute();
  $stmt->close();

  // get user id
  $stmt = $conn->prepare("SELECT ID_U FROM Users WHERE Email=?");
  $stmt->bind_param("s", $email);
  $stmt->execute();
  $user_id_r = $stmt->get_result();
  $stmt->close();

  $conn->close();

  if($user_id_r === false) {
    trigger_error('Func: create_account_db. Logic - $user_id_r is false after successfully inserting a new user. It should be the user\'s new ID', E_USER_WARNING);
    die('Difficulties are being experienced creating your account. Contact us at mail@streamer.online and we\'ll fix it.');
  }

  if( $user_id_r->num_rows !== 1) {
    trigger_error('Func: create_account_db. Logic - $user_id_r->num_rows is not 1. It should be 1', E_USER_WARNING);
    die('Difficulties are being experienced creating your account. Contact us at mail@streamer.online and we\'ll fix it.');
  }

  if( ($user_id = $user_id_r->fetch_assoc() ) === NULL ) {
    trigger_error('Func: create_account_db. Logic - $user_id_r->fetch_assoc() returns NULL. IT should return the new user\'s ID.', E_USER_WARNING);
    die('Difficulties are being experienced creating your account. Contact us at mail@streamer.online and we\'ll fix it.');
  }

  return $user_id['ID_U'];
}

// create account
function create_account($name, $email, $pwd_1, $pwd_2, $r_key) {

    // NAME

    // empty - Please enter a name
    if( !isset($name) || empty($name) ) {
      die("Please enter a name");
    }
    // Your name can only be 1 to 32 characters long
    if( !preg_match('/^.{1,32}$/' , $name) ) {
      die("Your name can only be between 1 and 32 characters long");
    }
    // Your name can only contain letters, numbers, spaces, - and '
    if( !preg_match('/^[a-zA-Z\-\s\']+$/' , $name) ) {
      die("Your name can only contain letters, numbers, spaces, - and '");
    }

    // EMAIL
    if( !isset($email) || empty($email) ) {
      die("Please enter an email");
    }
    if( strlen($email) > 255 ) {
      die("Your email can only be up to 255 characters long");
    }
    // email is valid (xxx@xxx.xxx)(255 limit)
    if( !preg_match('/^([\w-]+(?:\.[\w-]+)*)@((?:[\w-]+\.)*\w[\w-]{0,66})\.([a-z]{2,6}(?:\.[a-z]{2})?)$/', $email) ) {
        die("Your email is not valid");
    }
    // invalid - Invalid email address
      //@
      //.
      //uname
      //domain
      //tld
    // exists - This email is already registered
    if( email_exists($email) ) {
      die("This email is already registered");
    }

    // PWD

    // empty
    if( !isset($pwd_1) || empty($pwd_1) ) {
      die("Please enter a password");
    }
    // length - Please enter a password between 8 and 64 characters
    // invalid - Your email must contain at least 1 lowercase, uppercase, and number/special character
    if( !isset($pwd_1) || empty($pwd_1) || //strlen($pwd_1) < 6 || strlen($pwd_1) > 64 ) {
        !preg_match('/(?=^.{8,64}$)((?=.*\d)|(?=.*\W+))(?![.\n])(?=.*[A-Z])(?=.*[a-z]).*$/', $pwd_1) ) {
        die("Password must contain 8 - 64 lowercase, uppercase, and number/special characters");
    }


    // pwds match
    if( $pwd_1 !== $pwd_2) {
        die("Passwords don't match");
    }

    // robot
    if( client_robot($r_key) ) {
      die("Please complete the reCaptcha");
    }

    // hash password
    if( ($pwd_hash = password_hash($pwd_1, PASSWORD_DEFAULT) ) === false) {
      trigger_error('Func: create_account. HASH - password hash could not be created. password_hash() returned false.', E_USER_WARNING);
      die('Sorry, your account could not be created');
    }

    $user_id = create_account_db($name, $email, $pwd_hash);

    // create user tracks directory
    $DOCUMENT_ROOT = $_SERVER['DOCUMENT_ROOT'];
    $loc = strrpos($DOCUMENT_ROOT, "/");
    $tracks_dir = substr($DOCUMENT_ROOT, 0, $loc+1).'site_content/user_tracks/'.$user_id;
    if( !mkdir($tracks_dir, 0744) ) {
      trigger_error('Func: create_account. MKDIR - track directory could not be created.', E_USER_WARNING);
      die("An unexpected error occured setting up your account. Contact mail@streamer.online for assistance.");
    }

    setup_email_validation($user_id, $email);

    // set session variables
    $_SESSION['loggedIn'] = true;
    $_SESSION['user_id'] = $user_id;
    $_SESSION['user_email'] = $email;
  }

// setup_email_validation
function setup_email_validation($user_id, $email) {
  // create EmailValidation row
  $ev_hash = md5( rand(0,1000) );

  $conn = new mysqli('localhost', 'insert_emailv', 'obscure', 'streamer'); // db data
  if ($conn->connect_error) {
      trigger_error('Func: setup_email_validation. MYSQL Connection failed: '.$conn->connect_error, E_USER_WARNING);
      die('An unexpected error occured setting up your account');
  }

  // prepare and bind
  if ( !($stmt = $conn->prepare("INSERT INTO EmailValidation (ID_U, Hash) VALUES (?, ?)") ) ) {
    trigger_error('Func: setup_email_validation. MYSQL - INSERT EmailValidation: prepare error', E_USER_WARNING);
    die("An unexpected error occured setting up your account");
  }
  if ( !($stmt->bind_param("is", $user_id, $ev_hash) ) ) {
    trigger_error('Func: setup_email_validation. MYSQL - INSERT EmailValidation: bind_param error', E_USER_WARNING);
    die("An unexpected error occured setting up your account");
  }
  $stmt->execute();
  $stmt->close();

  // close db
  $conn->close();

  // send valid. email
  $to      = $email; // Send email to our user
  $subject = 'Streamer Email Verification'; // Give the email a subject
  $message = '

  Thanks for signing up!
  Your account has been created, click the link below to activate your account and begin uploading music!

  Please click this link to activate your account:
  https://www.cameronrowemusic.com/verify.php?email='.urlencode($email).'&hash='.$ev_hash.'

  ';

  $headers = 'From:mail@streamer.online' . "\r\n"; // Set from headers
  if( !mail($to, $subject, $message, $headers) ) {
    trigger_error('Func: setup_email_validation. mail failed to send to recipient: '.$to, E_USER_WARNING);
    die("An unexpected error occured setting up your account");
  }

}

// search tracks
function search_tracks($str) {
  // Create connection
  $conn = new mysqli('localhost', GET_TRACKS_USR, GET_TRACKS_PWD, 'streamer'); // db data
  if ($conn->connect_error) {
      trigger_error('Func: search_tracks. MYSQL Connection failed: '.$conn->connect_error, E_USER_ERROR);
      die();
  }

  // search db for matches (limit?)
  if ( !($stmt = $conn->prepare("SELECT ID_T, Filename, Title, Artist, Length FROM Tracks WHERE ID_U=? AND (Filename LIKE ? OR Title LIKE ? OR Artist LIKE ? OR Mix LIKE ? OR Year LIKE ? OR Genre LIKE ?)") ) ) {
    trigger_error('Func: search_tracks. MYSQL - SELECT User: prepare error', E_USER_ERROR);
    die();
  }
  if ( !($stmt->bind_param("issssss", $_SESSION['user_id'], $str, $str, $str, $str, $str, $str) ) ) {
    trigger_error('Func: search_tracks. MYSQL - SELECT User: bind_param error', E_USER_ERROR);
    die();
  }
  if( !$stmt->execute() ) {
    trigger_error('Func: search_tracks. MYSQL - SELECT User: execute error', E_USER_ERROR);
    die();
  }
  if ( !($result = $stmt->get_result() ) ) {
    trigger_error('Func: search_tracks. MYSQL - SELECT User: get_result error', E_USER_ERROR);
    die();
  }
  $stmt->close();

  $conn->close();


  // get results
  $result_arr = array();

  for($i = 0; $i < $result->num_rows; $i++) {
      $row = $result->fetch_assoc();

      $result_arr[$i]['ID_T'] = $row['ID_T'];

      if( $row['Title'] ) {
        $result_arr[$i]['Title'] = htmlspecialchars($row['Title']);
      } else {
        $result_arr[$i]['Title'] = htmlspecialchars($row['Filename']);
      }

      $result_arr[$i]['Artist'] = htmlspecialchars($row['Artist']);

      $result_arr[$i]['Length'] = htmlspecialchars($row['Length']);
  }

  // send json to client (including track ids)
  return json_encode($result_arr);
}

// get track filedir
function get_track_filedir($track_id) {
  // search user database for track (url)
  // Create connection
  $conn = new mysqli('localhost', GET_TRACKS_USR, GET_TRACKS_PWD, 'streamer'); // db data
  if ($conn->connect_error) {
      trigger_error('Func: get_track_filedir. MYSQL Connection failed: '.$conn->connect_error, E_USER_ERROR);
      die();
  }

  // search db for matches (limit?)
  if ( !($stmt = $conn->prepare("SELECT Filename FROM Tracks WHERE ID_U=? AND ID_T=?") ) ) {
    trigger_error('Func: get_track_filedir. MYSQL - SELECT Track: prepare error', E_USER_ERROR);
    die();
  }
  if ( !($stmt->bind_param("ii", $_SESSION['user_id'], $track_id) ) ) {
    trigger_error('Func: get_track_filedir. MYSQL - SELECT Track: bind_param error', E_USER_ERROR);
    die();
  }
  if( !$stmt->execute() ) {
    trigger_error('Func: get_track_filedir. MYSQL - SELECT Track: execute error', E_USER_ERROR);
    die();
  }
  if ( !($result = $stmt->get_result() ) ) {
    trigger_error('Func: get_track_filedir. MYSQL - SELECT Track: get_result error', E_USER_ERROR);
    die();
  }
  $stmt->close();

  $conn->close();

  if($result->num_rows !== 1) {
    trigger_error('Func: get_track_filedir. MYSQL - SELECT Track: wrong num of rows for track, should be 1.', E_USER_ERROR);
    die();
  }

  if( ($row = $result->fetch_assoc() ) === NULL ) {
    trigger_error('Func: get_track_filedir. MYSQL - SELECT Track: row is null.', E_USER_ERROR);
    die();
  }

   $filedir = dirname($_SERVER['DOCUMENT_ROOT']).'/site_content/user_tracks/'.$_SESSION['user_id'].'/'.$row['Filename'];

   return $filedir;
}

// index check
function index_check($site_content_dir) {
  if( isset($_SESSION['loggedIn']) && !empty($_SESSION['loggedIn']) && $_SESSION['loggedIn'] ) {
    $page = file_get_contents($site_content_dir.'player.html');
    if($page === false) {
      echo 'Sorry, the page cannot be displayed';
      trigger_error('Func: index_check. file_get_contents fail: player.html cannot be read', E_USER_ERROR);
      die();
    }
    $page = str_replace("<title></title>", '<title>Streamer - Player - '.htmlspecialchars($_SESSION['user_email']).'</title>', $page);
  } else {
    $page = file_get_contents($site_content_dir.'login.html');
    if($page === false) {
      echo 'Sorry, the page cannot be displayed';
      trigger_error('Func: index_check. file_get_contents fail: login.html cannot be read', E_USER_ERROR);
      die();
    }
  }
  return $page;
}

// logout
function logout() {
  $_SESSION['loggedIn'] = false;
  $_SESSION['user_id'] = undefined;
  $_SESSION['user_email'] = undefined;
}

// upload tracks
function upload_tracks($file, $upload_id) {

  // check for error
  if ($file['error'] !== UPLOAD_ERR_OK) {
    trigger_error('Func: upload_tracks. File Upload Error Message: '.get_upload_error_message($file['error']), E_USER_ERROR);
    die();
  }

  // MIME
  if( $file['type'] !== 'audio/mp3' && $file['type'] !== 'audio/mpeg' ) {
    trigger_error('Func: upload_tracks. Bad file type: '.$file['type'], E_USER_ERROR);
    die();
  }

  // size
  if( $file['size'] === 0) {
    trigger_error('Func: upload_tracks. file size is 0');
    die('File size is 0');
  } else if($file['size'] > 1000000000) {
    trigger_error('Func: upload_tracks. file size is too large. Max file size: 1GB');
    die('File too large. Max file size: 1GB');
  }

  // Name
  // greater than 0
  if( strlen($file['name']) === 0) {
    die('set new filename');
  }
  // less than 256
  if( strlen($file['name']) > 255) {
    die('shorten filename');
  }

  // filename invalid
  if( strrpos($file['name'] , '\0') !== false ) {
    die('The filename cannot contain NULL');
  }
  if( strrpos($file['name'] , '/') !== false ) {
    die('The filename cannot contain /');
  }

  // not exist in db for user
  if( ($filename = validate_track_filename($_SESSION['user_id'], basename($file['name']) ) ) === false ) {
    trigger_error('Func: upload_tracks. filename needs to be changed to seomthing unique');
    die('Change filename to something unique');
  }

  // save file
  $uploaddir = dirname($_SERVER['DOCUMENT_ROOT']).'/site_content/user_tracks/'.$_SESSION['user_id'].'/';
  $uploadfile = $uploaddir.$filename;

  if ( !move_uploaded_file($file['tmp_name'], $uploadfile)) {
      trigger_error('Func: upload_tracks. Fail upload fail. Possible file upload attack!. $uploadfile: '.$uploadfile.'. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
      die();
  }

  // get id3 tags
  $id3tags = get_ID3_tags($uploadfile);

  // save file in db
  $track_id = save_track_db($_SESSION['user_id'], $filename, $file, $id3tags);

  // echo id3 tags
  $out_arr = array(
      'upload_id' => $upload_id,
      'ID_T' => $track_id,
      'Title' => ($id3tags['Title'] === NULL ? $filename : $id3tags['Title']),
      'Artist' => $id3tags['Artist'],
      'Length' => $id3tags['Length']
  );

  return $out_arr;
}
?>
