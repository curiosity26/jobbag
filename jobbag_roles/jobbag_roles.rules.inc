<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 2/16/14
 * Time: 3:01 PM
 */

function jobbag_roles_rules_event_info() {
  $info = array();

  $info['job_role_user_added'] = array(
    'label' => t('A new user has been added to a job role'),
    'group' => t('Job'),
    'variables' => array(
      'role' => array(
        'label' => t('Job Role'),
        'type' => 'entity',
        'description' => t('The role the user was added to')
      ),
      'account' => array(
        'label' => t('Account'),
        'type' => 'user',
        'description' => t('The user account that was added')
      ),
      'job' => array(
        'label' => t('Job'),
        'type' => 'entity',
        'description' => t('The job the role belongs to'),
        'handler' => 'job_load_by_role'
      )
    )
  );

  $info['job_role_user_deleted'] = array(
    'label' => t('A new user has been removed from a job role'),
    'group' => t('Job'),
    'variables' => array(
      'role' => array(
        'label' => t('Job Role'),
        'type' => 'entity',
        'description' => t('The role the user was removed from')
      ),
      'account' => array(
        'label' => t('Account'),
        'type' => 'user',
        'description' => t('The user account that was added')
      ),
      'job' => array(
        'label' => t('Job'),
        'type' => 'entity',
        'description' => t('The job the role belongs to'),
        'handler' => 'job_load_by_role'
      )
    )
  );

  return $info;
}

function job_load_by_role($arguments, $name, $info) {
  $role = $arguments['role'];
  return job_load($role->jid);
}
