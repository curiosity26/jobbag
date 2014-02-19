<?php

function jobbag_role_job_form($form, &$form_state, $job) {
  $roles = jobbag_role_load_multiple();
  drupal_set_title(t("@job_number's Roles", array('@job_number' => $job->getJobNumber())));

  $form['role_users']['#tree'] = TRUE;

  $form['roles'] = array(
    '#type' => 'value',
    '#value' => $roles
  );

  foreach ($roles as $role) {
    $users = array();

    if (!empty($job->roles[$role->rid]) && !empty($job->roles[$role->rid]->users)) {
      $users = $job->roles[$role->rid]->users;
    }

    $form['role_name'][$role->rid] = array(
      '#type' => 'markup',
      '#markup' => $role->label
    );

    $eligible = array(0 => t('-- None --'));

    foreach (job_roles_eligible_users($role) as $account) {
      if (isset($account->uid) && isset($account->name)) {
        $eligible[$account->uid] = $account->name;
      }
    }
    /*
    if ($role->rid == 1) {
     unset($users[0]); // Anons not allowed
    }
    */

    $form['role_users'][$role->rid] = array(
      '#type' => 'select',
      '#multiple' => $role->cardinality != 1,
      '#options' => $eligible,
      '#default_value' => !empty($users) ? $users : NULL,
      '#size' => $role->cardinality == 1 ? 1 : min(10, count($users)),
      '#tree' => TRUE
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save roles')
  );

  return $form;
}

function jobbag_role_job_form_submit(&$form, &$form_state) {
  $job = $form_state['build_info']['args'][0];
  $success = FALSE; // It's not a success until the job is done.
  $hook_info = array(
    'user_added' => array(),
    'user_removed' => array()
  );
  if (is_object($job)) {
    $controller = entity_get_controller('job_role');
    foreach ($form_state['values']['role_users'] as $rid => $uids) {
      $role = $controller->loadByJob($job, array('rid' => $rid));
      if (!$role) {
        $role = entity_create('job_role', array('rid' => $rid, 'jid' => $job->identifier()));
        $hook_info['user_added'][$rid] = array('role' => $role, 'users' => $uids);
        dpm($hook_info);
      }
      elseif (count($uids) > count($role->users)) {
        $hook_info['user_added'][$rid] = array(
          'role' => $role,
          'users' => array_diff($uids, array_keys($role->users))
        );
      }
      elseif (count($uids) < count($role->users)) {
        $hook_info['user_deleted'][$rid] = array(
          'role' => $role,
          'users' => array_diff(array_keys($role->users), $uids)
        );
      }

      $role->setUsers((array)$uids);
      $success = entity_save('job_role', $role);

      if ($success === FALSE) {
        form_error($form['role_users'][$rid], 'Unable to save job role settings');
        $form_state['rebuild'] = TRUE;
        break;
      }
    }

    if ($success !== FALSE) {
      drupal_set_message('Successfully saved job role settings');
      job_load_roles($job, TRUE);
    }

    // Rules Rule
    foreach ($hook_info as $hook => $roles) {
      foreach ($roles as $rid => $info) {
        foreach ($info['users'] as $uid) {
          $controller->invoke($hook, $info['role'], user_load($uid));
        }
      }
    }
  }
}