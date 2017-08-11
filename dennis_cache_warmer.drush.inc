<?php

/**
 * @file
 * drush commands
 */

/**
 * Implements hook_drush_command().
 */
function dennis_cache_warmer_drush_command() {
  $items = array();
  // The command options.
  $options = array(
    'paths' => 'Path to a file containing the paths to warm.',
    'autouser' => 'Generate a temporary user to visit pages.',
    'username' => 'The username with which to log into Drupal. Anonymous access is the default.',
    'password' => 'The password associated with the specified username. See "--username".',
  );
  // The provided commands.
  $items['dennis-cache-warm'] = array(
    'callback' => 'dennis_cache_warmer_run',
    'description' => 'Warms the cache.',
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
    'arguments' => array('site' => 'Site base URI.'),
    'examples' => array(
      'drush dennis-cache-warm --autouser http://auth.example.com'
        => 'Visits pages on the auth site.',
      'drush dennis-cache-warm --username=admin --password=password http://auth.example.com'
        => 'Visit pages with an existing admin account',
    ),
    'options' => $options,
    'aliases' => array('dcw'),
  );
  return $items;
}

/**
 * Crawl the URIs of the site specified starting at the given base URI.
 *
 * @param $base_uri string
 *   The base URI of the site being crawled.
 *
 * @return string
 *   The request responses, status and timeouts in JSON format.
 */
function dennis_cache_warmer_run($base_uri = '') {
  if (empty($base_uri)) {
    return drush_set_error('DENNIS_CACHE_WARMER_ERROR', 'A base url for the site is required.');
  }
  rtrim($base_uri, '/');

  // Read paths from an external file.
  $file = drush_get_option('paths');
  if (!empty($file)) {
    if (!file_exists($file)) {
      return drush_set_error('DENNIS_CACHE_WARMER_ERROR', 'Paths file not found.');
    }
    $paths = file($file);
    $paths = array_map('trim', $paths);
  }
  else {
    $paths = array('/', '/news', '/reviews', '/contact-us');
  }

  $credentials = array();

  // Generate a temporary admin user to bypass the maintenance mode.
  if (drush_get_option('autouser')) {
    // Create user
    $name = 'dennis_cache_warmer_'
      . chr(rand(65, 90)) . chr(rand(65, 90)) . chr(rand(65, 90)) . chr(rand(65, 90)) . chr(rand(65, 90))
      . '_' . time();
    $password = chr(rand(65, 90)) . chr(rand(65, 90)) . chr(rand(65, 90)) . chr(rand(65, 90)) . chr(rand(65, 90))
      . rand(10, 99);

    $options = array(
      'password' => $password,
      'mail' => $name . '@example.com',
    );
    drush_invoke_process('@self', 'user-create', array($name), $options);
    drush_invoke_process('@self', 'user-add-role', array(
      'administrator',
      $name
    ));
    $credentials = array('username' => $name, 'password' => $password);
  }
  else {
    // Get the credentials for logging in, if available.
    $username = drush_get_option('username');
    $password = drush_get_option('password');
    if (!empty($username) && !empty($password)) {
      $credentials = array('username' => $username, 'password' => $password);
    }
  }

  try {
    $results = dennis_cache_warmer_visit($base_uri, $paths, $credentials);
    if (empty($results)) {
      return drush_set_error('DENNIS_CACHE_WARMER_ERROR', 'ERROR: No urls visited.');
    }
    foreach ($results as $result) {
      drush_print($result);
    }
  } catch (Exception $e) {
    return drush_set_error('DENNIS_CACHE_WARMER_ERROR', 'ERROR: ' . $e->getMessage());
  }

  // Remove the auto created user.
  if (isset($name)) {
    $options = array(
      'yes' => NULL,
      'delete-content' => NULL,
    );
    drush_invoke_process('@self', 'user-cancel', array($name), $options);
  }
}
