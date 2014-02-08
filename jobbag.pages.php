<?php

function jobbag_type_form($form, &$form_state, $op = 'edit', $type = NULL, $job = NULL) {
  $clients = db_select('jobbag_client', 'jc')->fields('jc', array('cid', 'label'))->execute()->fetchAllAssoc('cid');

  if (empty($clients)) {
    $form['empty'] = array(
      '#markup' => t('<p align="center">No Clients Entered in the System. </p>', array('!link' => l('Add a Client', 'admin/jobs/clients/add')))
    );

    return $form;
  }

  if (empty($job)) {
    $entity_values = array(
      'type' => isset($type) ? $type : 'default',
      'user' => $GLOBALS['user']
    );
    $job = entity_create('job', $entity_values);
  }

  $form_state['job'] = $job;

  $options = array();

  foreach ($clients as $client) {
    $options[$client->cid] = $client->label;
  }

  $form['client_id'] = array(
    '#type' => 'select',
    '#title' => t('Select a Client'),
    '#required' => TRUE,
    '#options' => array('--' => '-- Select a Client --') + $options,
    '#default_value' => isset($job->client_id) ? $job->client_id : '--'
  );

  if ($op != 'add') {
    $form['client_id']['#disabled'] = TRUE;
  }

  $form['description'] = array(
    '#type' => 'textarea',
    '#resizable' => TRUE,
    '#title' => t('Description'),
    '#default_value' => isset($job->description) ? $job->description : ''
  );

  $form['status_options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Status'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );

  $form['status_options']['status'] = array(
    '#type' => 'radios',
    '#title' => 'Status',
    '#title_display' => 'invisible',
    '#options' => job_status(),
    '#default_value' => isset($job->status) ? $job->status : 1
  );

  $form['operations']['add'] = array(
    '#type' => 'submit',
    '#value' => $op == 'add' ? t('Create job') : t('Save job'),
  );

  if ($op != 'add') {
    $form['operations']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => 'jobbag_type_form_delete'
    );
  }

  return $form;
}

function jobbag_type_form_submit($form, &$form_state) {
  $job = $form_state['job'];
  $values = $form_state['values'];
  $job->description = $values['description'];
  $job->status = $values['status'];
  entity_save($job->entityType(), $job);
  $dest = $_GET['destination'];
  if (!$dest) {
    $dest = 'job/'.$job->jid;
  }
  $form_state['redirect'] = $dest;
}

function jobbag_type_form_delete($form, &$form_state) {
  $form_state['redirect'] = 'job/'.$form_state['job']->jid.'/delete';
}

function jobbag_delete($form, &$form_state, $job) {
  $form_state['job'] = $job;
  $dest = $_GET['destination'];
  if (!$dest) {
      $dest = 'job/'.$job->jid;
  }
  return confirm_form($form, t('Are you sure you want to delete !title?', array('!title' => $job->title)), $dest);
}

function jobbag_delete_submit($form, &$form_state) {
  $job = $form_state['job'];
  entity_delete('job', $job->jid);
  $form_state['redirect'] = '<front>';
}