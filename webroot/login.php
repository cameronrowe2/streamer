<?php

session_start();

$req_dir = dirname($_SERVER['DOCUMENT_ROOT']).'//site_content//';
require_once $req_dir.'defs.inc';

setup_err_funcs();

$ip = $_SERVER['REMOTE_ADDR'];
$email = $_POST['email'];
$pwd = $_POST['pwd'];

login($ip, $email, $pwd);

?>
