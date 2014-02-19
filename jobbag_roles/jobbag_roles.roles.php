<?php

function jobbag_role_job_form($form, &$form_state, $job) {
  $roles = jobbag_role_load_multiple();
  $form_state['storage']['entity'] = $job;
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
  $job = $form_state['storage']['entity'];
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

      $role->users = !is_array($uids) ? array($uids) : $uids;
      $success = entity_save('job_role', $role);
      /*$jrid = db_select('jobbag_job_roles', 'r')
        ->distinct()
        ->fields('r', array('jrid'))
        ->condition('jid', $job->identifier())
        ->condition('rid', $rid)
        ->execute()
        ->fetchField();

      $record = new stdClass();
      $key = array();

      if ($jrid) {
          $key[] = 'jrid';
          $record->jrid = $jrid;
      }

      $record->rid = $rid;
      $record->jid = $job->jid;
      $record->users = !is_array($uids) ? array($uids) : $uids;

      $success = drupal_write_record('jobbag_job_roles', $record, $key);

      unset($jrid);
      unset($record);
      unset($key);*/

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
    foreach ($hook_info as $hook => $info) {
      foreach ($info['users'] as $uid) {
        $controller->invoke($hook, $info['role'], user_load($uid));
      }
    }
  }
}