<?php

session_start();

$req_dir = dirname($_SERVER['DOCUMENT_ROOT']).'//site_content//';
require_once $req_dir.'defs.inc';

setup_err_funcs();

$page = index_check('');
echo $page;
?>
