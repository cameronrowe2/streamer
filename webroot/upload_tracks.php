<?php

session_start();

$req_dir = dirname($_SERVER['DOCUMENT_ROOT']).'//site_content//';
require_once $req_dir.'defs.inc';

setup_err_funcs();

$file = $_FILES['file'];

$upload_id = $_POST['upload_id'];
if( !preg_match('/^[0-9]+$/', $upload_id) || $upload_id !== strval(intval($upload_id)) ) {
  trigger_error('$upload_id incorrect format', E_USER_ERROR);
  die();
}

$out_arr = upload_tracks($file, $upload_id);

ob_clean();
ob_end_flush();
echo json_encode($out_arr);

?>
