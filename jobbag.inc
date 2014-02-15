<?php

function jobbag_type_page() {
  $types = jobbag_get_types();
  $items = array();
  foreach ($types as $type) {
    $items[] = l($type->label, 'job/add/'.$type->type);
  }

  return theme('item_list', array('items' => $items, 'attributes' => array('class' => 'admin-list')));
}

function jobbag_type_form($form, &$form_state, $op = 'edit', $type = NULL, $job = NULL) {
  $clients = db_select('jobbag_client', 'jc')
    ->fields('jc', array('cid', 'label'))
    ->condition('status', '0', '!=')
    ->execute()
    ->fetchAllAssoc('cid');

  if (empty($clients)) {
    $message = t('<p align="center">No Clients Active in the System. </p>',
      array('!link' => l('Add a Client', 'admin/jobs/clients/add')));
    drupal_set_message($message, 'warning');

    return $form;
  }

  if (empty($job)) {
    $entity_values = array(
      'type' => isset($type) ? $type : 'default',
      'user' => $GLOBALS['user']
    );
    $job = entity_create('job', $entity_values);
  }

  if ($op == 'add') {
    drupal_set_title('Create a New Job');
  }

  if ($op == 'edit') {
    drupal_set_title('Edit '.$job->getJobNumber());
  }

  $form_state['job'] = $job;

  // Attach any fields
  $langcode = isset($GLOBALS['user']->language) ? $GLOBALS['user']->language : $GLOBALS['language']->language;
  field_attach_form('job', $job, $form, $form_state, $langcode);

  if ($op != 'add') {
    $form['field_job_client']['#disabled'] = TRUE;
  }

  $form['settings'] = array(
    '#type' => 'vertical_tabs',
    '#default_tab' => 'reminder_settings'
  );

  $form['reminder_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Reminder'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#group' => 'settings',
    '#tree' => TRUE
  );

  $form['reminder_settings']['reminder_override'] = array(
    '#type' => 'checkbox',
    '#title' => t('@override reminders for this job', array('@override' =>
      variable_get('job_reminder_disabled', FALSE) ? 'Enable' : 'Disable')),
    '#default_value' => isset($job->data['reminder_override'])
        ? $job->data['reminder_override']
        : FALSE
  );

  $form['reminder_settings']['reminder_date'] = array(
    '#type' => 'actions',
    '#tree' => TRUE,
    '#element_suffix' => t('before due date')
  );

  $form['reminder_settings']['reminder_date']['metric'] = array(
    '#type' => 'textfield',
    '#size' => 3,
    '#maxlength' => 2,
    '#default_value' => $job->getReminderMetric()
  );

  $form['reminder_settings']['reminder_date']['unit'] = array(
    '#type' => 'select',
    '#options' => array(
      'i' => t('Minute(s)'),
      'g' => t('Hour(s)'),
      'j' => t('Day(s)')
    ),
    '#default_value' => $job->getReminderUnit()
  );

  $form['date_due_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Date Due'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#group' => 'settings',
    '#tree' => TRUE
  );

  $form['date_due_settings']['date_due_override'] = array(
    '#type' => 'checkbox',
    '#title' => t('@override date due notification for this job', array('@override' =>
      variable_get('job_date_due_disabled', FALSE) ? 'Enable' : 'Disable')),
    '#default_value' => isset($job->data['date_due_override'])
        ? $job->data['date_due_override']
        : FALSE
  );

  $form['past_due_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Past Due'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#group' => 'settings',
    '#tree' => TRUE
  );

  $form['past_due_settings']['past_due_override'] = array(
    '#type' => 'checkbox',
    '#title' => t('@override past due notification for this job', array('@override' =>
      variable_get('job_past_due_disabled', FALSE) ? 'Enable' : 'Disable')),
    '#default_value' => isset($job->data['past_due_override'])
        ? $job->data['past_due_override']
        : FALSE
  );

  $form['closed_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Closed Notification'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#group' => 'settings',
    '#tree' => TRUE
  );

  $form['closed_settings']['closed_override'] = array(
    '#type' => 'checkbox',
    '#title' => t('@override closed notification for this job', array('@override' =>
      variable_get('job_closed_disabled', FALSE) ? 'Enable' : 'Disable')),
    '#default_value' => isset($job->data['closed_override'])
        ? $job->data['closed_override']
        : FALSE
  );

  $form['cancelled_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Cancelled Notification'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#group' => 'settings',
    '#tree' => TRUE
  );

  $form['cancelled_settings']['cancelled_override'] = array(
    '#type' => 'checkbox',
    '#title' => t('@override cancelled notification for this job', array('@override' =>
      variable_get('job_cancelled_disabled', FALSE) ? 'Enable' : 'Disable')),
    '#default_value' => isset($job->data['cancelled_override'])
        ? $job->data['cancelled_override']
        : FALSE
  );

  $form['status_options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Status'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#group' => 'settings',
    '#tree' => TRUE
  );

  $form['status_options']['status'] = array(
    '#type' => 'radios',
    '#title' => t('Status'),
    '#title_display' => 'invisible',
    '#options' => job_status(),
    '#default_value' => isset($job->status) ? $job->status : 1
  );

  $form['actions'] = array(
    '#type' => 'actions'
  );

  $form['actions']['add'] = array(
    '#type' => 'submit',
    '#value' => $op == 'add' ? t('Create job') : t('Save job'),
    '#weight' => 49
  );

  if ($op != 'add' && jobbag_access('delete', $job)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => 'jobbag_type_form_delete',
      '#weight' => 50
    );
  }

  return $form;
}

function jobbag_type_form_validate(&$form, &$form_state) {
  $job = $form_state['job'];
  field_attach_form_validate('job', $job, $form, $form_state);
}

function jobbag_type_form_submit($form, &$form_state) {
  $job = $form_state['job'];
  $controller = entity_get_controller($job->entityType());
  $values = $form_state['values'];
  $job->status = $values['status_options']['status'];
  // Process notification settings
  $job->data['reminder_override'] = $values['reminder_settings']['reminder_override'];

  if ($job->data['reminder_override'] == TRUE) {
    $job->setReminder($values['reminder_settings']['reminder_date']['metric'],
      $values['reminder_settings']['reminder_date']['unit']);
  }

  $job->data['date_due_override'] = $values['date_due_settings']['date_due_override'];
  $job->data['past_due_override'] = $values['past_due_settings']['past_due_override'];
  $job->data['closed_override'] = $values['closed_settings']['closed_override'];
  $job->data['cancelled_override'] = $values['cancelled_settings']['cancelled_override'];

  // PreSave Field Data
  field_attach_presave('job', $job);

  // Save Field Data
  field_attach_submit('job', $job, $form, $form_state);

  $client = array_shift($values['job_client_id']);
  if (empty($job->client)) {
    $job->client_id = $client[0]['target_id'];
  }

  if ($job->status == 0) {
    $job->cancelled = REQUEST_TIME;
    $controller->invoke('cancelled', $job);
    $actions = trigger_get_assigned_actions('jobbag_cancelled');
    if ($actions) {
      actions_do($actions, $job);
    }
  }
  else {
    $job->cancelled = 0;
  }

  if ($job->status == 2){
    $job->closed = REQUEST_TIME;
    $controller->invoke('closed', $job);
    $actions = trigger_get_assigned_actions('jobbag_closed');
    if ($actions) {
      actions_do($actions, $job);
    }
  }
  else {
    $job->closed = 0;
  }
  entity_save($job->entityType(), $job);
  $form_state['redirect'] = isset($_GET['destination']) ?
    drupal_get_destination() : 'job/'.$job->identifier();
}

function jobbag_type_form_delete($form, &$form_state) {
  $form_state['redirect'] = 'job/'.$form_state['job']->jid.'/delete';
}

function jobbag_delete($form, &$form_state, $job) {
  $form_state['job'] = $job;
  $dest = $_GET['destination'];
  if (!$dest) {
    $dest = 'job/'.$job->identifier();
  }
  return confirm_form($form, t('Are you sure you want to delete !title?', array('!title' => $job->title)), $dest);
}

function jobbag_delete_submit($form, &$form_state) {
  $job = $form_state['job'];
  entity_delete($job->entityType(), $job);
  $form_state['redirect'] = '<front>';
}