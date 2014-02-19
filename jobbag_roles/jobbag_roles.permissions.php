<?php

function job_permissions_job_form($form, &$form_state, $job) {
  $form_state['storage']['entity'] = $job;
  $perms = job_role_perms();
  $roles = jobbag_role_load_multiple();
  $controller = entity_get_controller('job_role');

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
      '#description' => isset($perm['description']) ? t($perm['description']) : '',
      '#tree' => TRUE
    );

    $form['checkboxes'][$name]['#tree'] = TRUE;

    // Add columns for each role
    foreach ($roles as $role) {
      if (($tmp = $controller->loadByJob($job, array('rid' => $role->rid))) !== FALSE) {
        dpm($tmp);
        $role = $tmp;
      }
      $form['checkboxes'][$name][$role->rid] = array(
        '#tree' => TRUE,
        '#type' => 'checkbox',
        '#return_value' => $name,
        '#default_value' => job_role_has_permission($name, $role) ? $name : FALSE
      );

      if ($role->rid == 1) {
        $form['checkboxes'][$name][$role->rid]['#disabled'] = TRUE;
        $form['checkboxes']['full access'][$role->rid]['#value'] =
          $form['checkboxes']['full access'][$role->rid]['#default_value'] = 'full access';
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
  $success = FALSE; // Can't be successful until the job is done

  $values = $form_state['values'];
  foreach ($values['checkboxes'] as $roles) {
    foreach($roles as $rid => $perm) {
      $perms[$rid][] = $perm;
    }
  }

  foreach ($perms as $rid => $permissions) {
    $role = $controller->loadByJob($job, array('rid' => $rid));
    if (!$role) {
      $role = entity_create('job_role', array('rid' => $rid, 'jid' => $job->identifier()));
    }

    $role->permissions = (array)$permissions;

    $success = entity_save('job_role', $role);

    if ($success === FALSE) {
      form_error($form['role_users'][$rid], 'Unable to save job permission settings');
      $form_state['rebuild'] = TRUE;
      break;
    }
  }
  if ($success !== FALSE) {
    drupal_set_message('Successfully saved job permission settings');
    job_load_roles($job, TRUE);
  }

}