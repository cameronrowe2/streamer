<?php

session_start();

if( !( $_SESSION['loggedIn'] && isset($_SESSION['user_id']) ) ) {
  trigger_error('Attempt made to stream a track from a non-logged in user. ip: '.$_SERVER['REMOTE_ADDR'], E_USER_WARNING);
  header('Location:index.php');
  exit;
}

$req_dir = dirname($_SERVER['DOCUMENT_ROOT']).'//site_content//';
require_once $req_dir.'defs.inc';
require_once('getid3/getid3.php');

setup_err_funcs();

// assign user and track id (GET)
$track_id = $_GET['track_id'];
if( !preg_match('/^[0-9]+$/', $track_id) || $track_id !== strval(intval($track_id)) ) {
  trigger_error('$track_id incorrect format. U_ID: '.$_SESSION['user_id'], E_USER_WARNING);
  exit;
}

$loc =  $_GET['loc'];
if( !preg_match('/^[0-9]+$/', $loc) || $loc !== strval(intval($loc)) || intval($loc) > 1000 ) {
  trigger_error('Track location ($loc) has incorrect format. U_ID: '.$_SESSION['user_id'], E_USER_WARNING);
  exit;
}

$loc = $loc/1000.0;

$filedir = get_track_filedir($track_id);

$getID3 = new getID3;
$fileInfo = $getID3->analyze($filedir);
getid3_lib::CopyTagsToComments($fileInfo);

if( !isset($fileInfo) ){
    trigger_error('$fileInfo not set after analysing file: '.$filedir.'. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
    die();
}
if( empty($fileInfo) ){
    trigger_error('$fileInfo empty after analysing file: '.$filedir.'. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
    die();
}

// file open
if( !($handle = fopen($filedir, 'r') ) ) {
  trigger_error('File error: fopen is null for file: '.$filedir.'. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
  die();
}

// seek
if( fseek($handle, $fileInfo['avdataoffset']) === -1) {
  trigger_error('File error: fseek error for file: '.$filedir.'. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
  die();
}

if( !($contents = fread($handle,  $fileInfo['avdataend'] - $fileInfo['avdataoffset'] ) ) ) {
  trigger_error('File error: fread is null for file: '.$filedir.'. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
  die();
}

if( !fclose($handle) ) {
  trigger_error('File error: fclose is null for file: '.$filedir.'. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
  die();
}

$length = strlen($contents);

if($loc !== 0) {
  $approx = round( ($length-1) * $loc );
  $start = closest_mp3_frame($contents, $approx);
  $length -= $start;
  $contents = substr($contents, $start, $length);
}

header('Content-Type: audio/mpeg' );
header('Content-Length: '.$length);
header("Content-Transfer-Encoding: binary");
header('Accept-Ranges: bytes');
ob_clean();
ob_end_flush();
print $contents;
exit;


// returns length of mp3 frame
function next_mp3_frame($data, $index){

  // A - frame sync
  if( ord($data[$index]) !== 255) {
    trigger_error('Func: next_mp3_frame. A1 err: '.ord($data[0]).' U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
    die();
  }

  $b2 = ord($data[$index+1]);
  $check = 128+64+32;
  if( ($b2 & $check ) !== $check ) {
    trigger_error('Func: next_mp3_frame. A2 err: '.$b2.' U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
    die();
  }

  // B
  $ver = ($b2 >> 3) & 3;

  switch($ver) {
    case 0:
      trigger_error('Func: next_mp3_frame. B err: ver: MPEG Version 2.5. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
      die();
      break;
    case 1:
      trigger_error('Func: next_mp3_frame. B err: ver: reserved. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
      die();
      break;
    case 2:
      trigger_error('Func: next_mp3_frame. B err: ver: MPEG Version 2 (ISO/IEC 13818-3). U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
      die();
      break;
    case 3:
      break;
    default:
      trigger_error('Func: next_mp3_frame. B err: non ver type. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
      die();
      break;
  }

  // C
  $lay = ($b2 >> 1) & 3;

  switch($lay) {
    case 0:
      trigger_error('Func: next_mp3_frame. C err: lay: reserved. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
      die();
      break;
    case 1:
      break;
    case 2:
      trigger_error('Func: next_mp3_frame. C err: ver:  Layer II. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
      die();
      break;
    case 3:
      trigger_error('Func: next_mp3_frame. C err: ver: Layer I. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
      die();
      break;
    default:
      trigger_error('Func: next_mp3_frame. C err: non lay type. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
      die();
      break;
  }

  // D
  $prot = $b2 & 1;

  // E
  $b3 = ord($data[$index+2]);
  $bitRate = ($b3 >> 4) & 15;

  switch($bitRate) {
    case 1:
      $bitRate = 32000;
      break;
    case 2:
      $bitRate = 40000;
      break;
    case 3:
      $bitRate = 48000;
      break;
    case 4:
      $bitRate = 56000;
      break;
    case 5:
      $bitRate = 64000;
      break;
    case 6:
      $bitRate = 80000;
      break;
    case 7:
      $bitRate = 96000;
      break;
    case 8:
      $bitRate = 112000;
      break;
    case 9:
      $bitRate = 128000;
      break;
    case 10:
      $bitRate = 160000;
      break;
    case 11:
      $bitRate = 192000;
      break;
    case 12:
      $bitRate = 224000;
      break;
    case 13:
      $bitRate = 256000;
      break;
    case 14:
      $bitRate = 320000;
      break;
    case 15:
      trigger_error('Func: next_mp3_frame. E err: bitrate: bad. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
      die();
    default:
      trigger_error('Func: next_mp3_frame. E err: bitrate: wrong. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
      die();
  }

  // F
  $sampleRate = ($b3 >> 2) & 3;

  switch($sampleRate) {
    case 0:
      $sampleRate = 44100;
      break;
    case 1:
      $sampleRate = 48000;
      break;
    case 2:
      $sampleRate = 32000;
      break;
    case 3:
      trigger_error('Func: next_mp3_frame. F err: smpRate: reserved. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
      die();
    default:
      trigger_error('Func: next_mp3_frame. F err: smpRate: wrong. U_ID: '.$_SESSION['user_id'], E_USER_ERROR);
      die();
  }

  // G
  $padding = ($b3 >> 1) & 1;

  // H - private bit

  // I - Channel mode

  // J - mode extension (only if joint stereo)

  // K - copyright

  // L - original

  // M - emphasis

  // calculate length

  $frameLen = intval(144*$bitRate/$sampleRate + $padding);

  return $frameLen;
}


function closest_mp3_frame($data, $loc2) {
  $prev = 0;
  $cumul = 0;

  while($cumul < $loc2) {
    $prev = $cumul;
    $cumul += next_mp3_frame($data, $cumul);
  }

  if( ($cumul-$loc2) < ($loc2-$prev) ) {
    return $cumul;
  }
  return $prev;

}

?>
