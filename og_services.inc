<?php

/*
 * Helper function for og_services_services_resources_alter.
 */
function _og_services_services_resources_alter(&$resources, $endpoint) {
  // [POST] {endpoint}/node/{nid}/join/{uid}
  $resources['node']['targeted actions']['join'] = array(
    'help' => 'Subscribe a user to a group',
    'file' => array('type' => 'inc', 'module' => 'og_services', 'name' => 'og_services'),
    'callback' => '_og_services_subscribe_user',
    'access callback' => 'user_access',
    'access arguments' => array('administer organic groups'),
    'access arguments append' => false,
    'args' => array(
      array(
        'name' => 'gid',
        'type' => 'int',
        'description' => 'A nid of a group on the system',
        'source' => array('path' => '0'),
        'optional' => false,
      ),
      array(
        'name' => 'uid',
        'type' => 'int',
        'description' => 'A uid of a user on the system',
        'source' => array('path' => '2'),
        'optional' => false,
      ),
    ),
  );
  // [POST] {endpoint}/node/{nid}/leave/{uid}
  $resources['node']['targeted actions']['leave'] = array(
    'help' => 'Unsubscribe a user to a group',
    'file' => array('type' => 'inc', 'module' => 'og_services', 'name' => 'og_services'),
    'callback' => '_og_services_unsubscribe_user',
    'access callback' => 'user_access',
    'access arguments' => array('administer organic groups'),
    'access arguments append' => false,
    'args' => array(
      array(
        'name' => 'gid',
        'type' => 'int',
        'description' => 'A nid of a group on the system',
        'source' => array('path' => '0'),
        'optional' => false,
      ),
      array(
        'name' => 'uid',
        'type' => 'int',
        'description' => 'A uid of a user on the system',
        'source' => array('path' => '2'),
        'optional' => false,
      ),
    ),
  );
  // [GET] {endpoint}/node/{nid}/users (optional) ?is_active=1&is_admin=1
  $resources['node']['relationships']['users'] = array(
    'help' => 'Show all users in a group',
    'file' => array('type' => 'inc', 'module' => 'og_services', 'name' => 'og_services'),
    'callback' => '_og_services_users',
    'access callback' => 'user_access',
    'access arguments' => array('administer organic groups'),
    'access arguments append' => false,
    'args' => array(
      array(
        'name' => 'is_active',
        'type' => 'int',
        'description' => 'Filter for active users',
        'source' => array('param' => 'is_active'),
        'optional' => true,
      ),
      array(
        'name' => 'is_admin',
        'type' => 'int',
        'description' => 'Filter for admins',
        'source' => array('param' => 'is_admin'),
        'optional' => true,
      ),
    ),
  );
  // [GET] {endpoint}/user/{uid}/groups (optional) ?is_active=1
  $resources['user']['relationships']['groups'] = array(
    'help' => 'Show groups user is a member of.',
    'file' => array('type' => 'inc', 'module' => 'og_services', 'name' => 'og_services'),
    'callback' => 'og_get_subscriptions',
    'access callback' => 'user_access',
    'access arguments' => array('administer organic groups'),
    'access arguments append' => false,
    'args' => array(
      array(
        'name' => 'is_active',
        'type' => 'int',
        'description' => 'Filter for active users',
        'source' => array('param' => 'is_active'),
        'optional' => true,
      ),
    ),
  );
}

/*
 * Join callback function that saves a user's subscription to a group.
 */
function _og_services_subscribe_user($gid, $uid) {
  return og_subscribe_user($gid, user_load($uid));
}

/*
 * Unjoin callback function.
 */
function _og_services_unsubscribe_user($gid, $uid) {
  global $user;
  $group_node = node_load($gid);
  $account = user_load($uid);
  og_delete_subscription($gid, $uid);
  // If needed, reload user object to reflect unsubscribed group.
  if ($user->uid == $account->uid) {
    og_get_subscriptions($account->uid, 1, TRUE); // Clear cache.
    $user = user_load(array('uid' => $user->uid));
    $message = t('You left the group %group.', array('%group' => $group_node->title));
  }
  else {
    $message = t('%user removed from %group.', array('%user' => $account->name, '%group' => $group_node->title));
  }
  return $message;
}

/**
 * List users in a group
 */
function _og_services_users($gid, $min_is_active = 1, $min_is_admin = 0) {
  $sql = og_list_users_sql($min_is_active, $min_is_admin);
  $result = db_query($sql, $gid);
  while ($row = db_fetch_array($result)) {
    $users[$row['uid']] = $row;
  }
  return $users;
}
