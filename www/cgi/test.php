<?php
header("Content-Type: text/plain");
echo "\n";
echo "REQUEST_METHOD: " . getenv('REQUEST_METHOD') . "\n";
echo "QUERY_STRING: " . getenv('QUERY_STRING') . "\n";
echo "CONTENT_LENGTH: " . getenv('CONTENT_LENGTH') . "\n";
echo "HTTP_HOST: " . getenv('HTTP_HOST') . "\n";
echo "Args: " . implode(' ', $argv) . "\n";
?>
