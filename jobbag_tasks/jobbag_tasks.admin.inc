<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 2/22/14
 * Time: 11:29 AM
 */

function jobbag_task_type_form($form, &$form_state, $jobbag_task_type, $op = 'edit') {
  if ($op == 'clone') {
    $jobbag_task_type->label .= ' (cloned)';
    $jobbag_task_type->type = '';
  }

  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $jobbag_task_type->label,
    '#description' => t('The human-readable name of this profile type.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  // Machine-readable type name.
  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($jobbag_task_type->type) ? $jobbag_task_type->type : '',
    '#maxlength' => 32,
    '#disabled' => $jobbag_task_type->isLocked() && $op != 'clone',
    '#machine_name' => array(
      'exists' => 'jobbag_task_type_exists',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this job type. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 40,
  );

  if (!$jobbag_task_type->isLocked() && $op != 'add' && $op != 'clone') {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#weight' => 45,
      '#limit_validation_errors' => array(),
      '#submit' => array('jobbag_task_type_form_submit_delete')
    );
  }
  return $form;
}

function jobbag_task_type_exists($name) {
  $types = entity_load_multiple_by_name('jobbag_task_type', array($name));
  return $types && !empty($types);
}

/**
 * Form API submit callback for the type form.
 */
function jobbag_task_type_form_submit(&$form, &$form_state) {
  $jobbag_task_type = entity_ui_form_submit_build_entity($form, $form_state);
  // Save and go back.
  entity_save($jobbag_task_type->entityType(), $jobbag_task_type);
  $form_state['redirect'] = 'admin/structure/job_tasks';
}

/**
 * Form API submit callback for the delete button.
 */
function jobbag_task_type_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/job_tasks/manage/' . $form_state['job_type']->type . '/delete';
}