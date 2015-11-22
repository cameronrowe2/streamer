<?php

session_start();

$req_dir = dirname($_SERVER['DOCUMENT_ROOT']).'//site_content//';
require_once $req_dir.'defs.inc';

setup_err_funcs();

$outstr = '';

// get and santise search string
//$str = trim($_POST['string']);
//$str = "%{$str}%";

$outstr = search_tracks("%");

echo $outstr;
?>
