<?php

function job_permissions_job_form($form, &$form_state, $job) {
  $form_state['storage']['entity'] = $job;
  $perms = job_role_perms();
  $roles = jobbag_role_load_multiple();

  drupal_set_title(t("@job_number's Permissions", array('@job_number' => $job->getJobNumber())));

  $form['roles'] = array(
    '#type' => 'value',
    '#value' => $roles
  );

  $form['perms'] = array(
    '#type' => 'value',
    '#value' => $perms
  );

  $form['checkboxes']['#tree'] = TRUE;
  $form['permission']['#tree'] = TRUE;
  foreach ($perms as $name => $perm) {
    // Define the row for the permission
    $form['permission'][$name] =  array(
      '#type' => 'item',
      '#markup' => $perm['title'],
      '#description' => t($perm['description']),
      '#tree' => TRUE
    );

    $form['checkboxes'][$name]['#tree'] = TRUE;

    // Add columns for each role
    foreach ($roles as $role) {
      $form['checkboxes'][$name][$role->rid] = array(
        '#tree' => TRUE,
        '#type' => 'checkbox',
        '#return_value' => $name,
        '#default_value' => job_role_has_permission($name, $role) ? $name : FALSE
      );

      if ($role->rid == 1) {
        $form['checkboxes'][$name][$role->rid]['#disabled'] = TRUE;
        $form['checkboxes']['full access'][$role->rid]['#value'] = $form['checkboxes']['full access'][$role->rid]['#default_value'] = 'full access';
      }
    }
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save permissions'
  );

  return $form;
}

function job_permissions_job_form_submit($form, &$form_state) {
  $job = $form_state['storage']['entity'];
  $controller = entity_get_controller('job_role');
  $perms = array();

  $values = $form_state['values'];
  foreach ($values['checkboxes'] as $roles) {
    foreach($roles as $rid => $perm) {
      $perms[$rid][] = $perm;
    }
  }

  foreach ($perms as $rid => $permissions) {
    $role = $controller::loadByJob($job, array('rid' => $rid));
    if ($role) {
      $role = entity_create('job_role', array('rid' => $rid, 'jid' => $job->identifier()));
    }
    if (!is_array($permissions)) {
      $permissions = array($permissions);
    }

    $role->permissions = $permissions;

    $success = entity_save('job_role', $role);

    /*
    $jrid = db_select('jobbag_job_roles', 'r')
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
    $record->jid = $job->identifier();

    if (!is_array($permissions)) {
      $permissions = array($permissions);
    }

    $record->permissions = array_filter($permissions);

    $success = drupal_write_record('jobbag_job_roles', $record, $key);

    unset($jrid);
    unset($record);
    unset($key);*/

    if (!$success) {
      form_error($form['role_users'][$rid], 'Unable to save job permission settings');
      $form_state['rebuild'] = TRUE;
      break;
    }
  }

  drupal_set_message('Successfully saved job permission settings');

  job_load_roles($job, TRUE);
}