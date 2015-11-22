<?php


// get email and hash
// http://www.yourwebsite.com/verify.php?email='.$email.'&hash='.$hash.'

if( isset($_GET['email']) && !empty($_GET['email']) AND isset($_GET['hash']) && !empty($_GET['hash'])){
    // Verify data

    // if email exists and hash is correct
        // activate account
    // else
        // Invalid approach

}else{
    // Invalid approach
    header('Location:index.php');
    exit;
}

// send back to homepage (dont login) - only display dialog if success

?>
