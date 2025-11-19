<?php
echo "<h1>PHP CGI Test</h1>";

echo "<p>PHP is working!</p>";

echo "<h2>Server Information</h2>";
echo "<pre>";
print_r($_SERVER);
echo "</pre>";

echo "<h2>Environment Variables</h2>";
echo "<pre>";
print_r(getenv());
echo "</pre>";

echo "<h2>GET Parameters</h2>";
echo "<pre>";
print_r($_GET);
echo "</pre>";

echo "<h2>POST Parameters</h2>";
echo "<pre>";
print_r($_POST);
echo "</pre>";

echo "<h2>Uploaded Files</h2>";
echo "<pre>";
print_r($_FILES);
echo "</pre>";

