<?php

/* 
 * Admin Jobs Listing Section
 *
 */


function jobbag_list($form, &$form_state, $limit = 50) {
  if (isset($form_state['values']['operation']) && $form_state['values']['operation'] == 'delete') {
    return job_multiple_delete_confirm($form, $form_state, array_filter($form_state['values']['jobs']));
  }
  $form['filter'] = job_filter_form();
  $form['#submit'][] = 'job_filter_form_submit';
  $form['admin'] = jobbag_list_form($limit);

  return $form;
}

function job_filters() {
  $filters['status'] = array(
    'title' => t('status'),
    'options' => array(
      '[any]' => t('any'),
      'status-1' => t('active'),
      'status-2' => t('closed out'),
      'status-0' => t('cancelled'),
    ),
  );

  $filters['type'] = array(
    'title' => t('type'),
    'options' => array(
      '[any]' => t('any'),
    ) + jobbag_get_type_names(),
  );

  $filters['client'] = array(
    'title' => t('client'),
    'options' => array(
      '[any]' => t('any'),
    ) + job_client_get_names(),
  );

  return $filters;
}

function job_filter_form() {
  $session = isset($_SESSION['job_overview_filter']) ? $_SESSION['job_overview_filter'] : array();
  $filters = job_filters();

  $i = 0;
  $form['filters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Show only items where'),
    '#theme' => 'exposed_filters__node',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );

  foreach ($session as $filter) {
    list($type, $value) = $filter;
    $value = $filters[$type]['options'][$value];
    $t_args = array('%property' => $filters[$type]['title'], '%value' => $value);
    if ($i++) {
      $form['filters']['current'][] = array('#markup' => t('and where %property is %value', $t_args));
    }
    else {
      $form['filters']['current'][] = array('#markup' => t('where %property is %value', $t_args));
    }
    if (in_array($type, array('type', 'language'))) {
      // Remove the option if it is already being filtered on.
      unset($filters[$type]);
    }
  }

  $form['filters']['status'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('clearfix')),
    '#prefix' => ($i ? '<div class="additional-filters">' . t('and where') . '</div>' : ''),
  );

  $form['filters']['status']['filters'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('filters')),
  );

  foreach ($filters as $key => $filter) {
    $form['filters']['status']['filters'][$key] = array(
      '#type' => 'select',
      '#options' => $filter['options'],
      '#title' => $filter['title'],
      '#default_value' => '[any]',
    );
  }

  $form['filters']['status']['actions'] = array(
    '#type' => 'actions',
    '#attributes' => array('class' => array('container-inline')),
  );

  $form['filters']['status']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => count($session) ? t('Refine') : t('Filter'),
  );

  if (count($session)) {
    $form['filters']['status']['actions']['undo'] = array('#type' => 'submit', '#value' => t('Undo'));
    $form['filters']['status']['actions']['reset'] = array('#type' => 'submit', '#value' => t('Reset'));
  }

  drupal_add_js('misc/form.js');

  return $form;
}

function job_filter_form_submit($form, &$form_state) {
  $filters = job_filters();
  switch ($form_state['values']['op']) {
    case t('Filter'):
    case t('Refine'):
      // Apply every filter that has a choice selected other than 'any'.
      foreach ($filters as $filter => $options) {
        if (isset($form_state['values'][$filter]) && $form_state['values'][$filter] != '[any]') {
          // Flatten the options array to accommodate hierarchical/nested options.
          $flat_options = form_options_flatten($filters[$filter]['options']);
          // Only accept valid selections offered on the dropdown, block bad input.
          if (isset($flat_options[$form_state['values'][$filter]])) {
            $_SESSION['job_overview_filter'][] = array($filter, $form_state['values'][$filter]);
          }
        }
      }
      break;
    case t('Undo'):
      array_pop($_SESSION['job_overview_filter']);
      break;
    case t('Reset'):
      $_SESSION['job_overview_filter'] = array();
      break;
  }
}

function job_filter_form_query(SelectQueryInterface $query) {
  // Build query
  $filter_data = isset($_SESSION['job_overview_filter']) ? $_SESSION['job_overview_filter'] : array();
  foreach ($filter_data as $index => $filter) {
    list($key, $value) = $filter;
    switch ($key) {
      case 'status':
        // Note: no exploitable hole as $key/$value have already been checked when submitted
        list($key, $value) = explode('-', $value, 2);
      case 'type':
        $query->condition('j.' . $key, $value);
        break;
      case 'client':
        $query->condition('j.client_id', $value);
        break;
    }
  }
}

/**
 * Implements hook_node_operations().
 */
function jobbag_job_operations() {
  $operations = array(
    'activate' => array(
      'label' => t('Activate selected job'),
      'callback' => 'job_mass_update',
      'callback arguments' => array('updates' => array('status' => 1)),
    ),
    'close_out' => array(
      'label' => t('Close Out selected job'),
      'callback' => 'job_mass_update',
      'callback arguments' => array('updates' => array('status' => 2)),
    ),
	'cancel' => array(
      'label' => t('Cancel selected job'),
      'callback' => 'job_mass_update',
      'callback arguments' => array('updates' => array('status' => 0)),
    ),
    'delete' => array(
      'label' => t('Delete selected job'),
      'callback' => NULL,
    ),
  );
  return $operations;
}

function job_mass_update($jobs, $updates) {
  // We use batch processing to prevent timeout when updating a large number
  // of nodes.
  if (count($jobs) > 10) {
    $batch = array(
      'operations' => array(
        array('_job_mass_update_batch_process', array($jobs, $updates))
      ),
      'finished' => '_job_mass_update_batch_finished',
      'title' => t('Processing'),
      // We use a single multi-pass operation, so the default
      // 'Remaining x of y operations' message will be confusing here.
      'progress_message' => '',
      'error_message' => t('The update has encountered an error.'),
      // The operations do not live in the .module file, so we need to
      // tell the batch engine which file to load before calling them.
      'file' => drupal_get_path('module', 'jobbag') . '/jobbag.admin.inc',
    );
    batch_set($batch);
  }
  else {
    foreach ($jobs as $jid) {
      _job_mass_update_helper($jid, $updates);
    }
    drupal_set_message(t('The update has been performed.'));
  }
}

/**
 * Node Mass Update - helper function.
 */
function _job_mass_update_helper($jid, $updates) {
  $job = job_load($jid, NULL, TRUE);
  // For efficiency manually save the original node before applying any changes.
  $job->original = clone $job;
  foreach ($updates as $name => $value) {
    $job->$name = $value;
  }
  entity_save($job->entityType(), $job);
  return $job;
}

/**
 * Node Mass Update Batch operation
 */
function _job_mass_update_batch_process($jobs, $updates, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = count($jobs);
    $context['sandbox']['nodes'] = $jobs;
  }

  // Process nodes by groups of 5.
  $count = min(5, count($context['sandbox']['jobs']));
  for ($i = 1; $i <= $count; $i++) {
    // For each nid, load the node, reset the values, and save it.
    $jid = array_shift($context['sandbox']['jobs']);
    $job = _job_mass_update_helper($jid, $updates);

    // Store result for post-processing in the finished callback.
    $context['results'][] = l($job->title, 'job/' . $job->jid);

    // Update our progress information.
    $context['sandbox']['progress']++;
  }

  // Inform the batch engine that we are not finished,
  // and provide an estimation of the completion level we reached.
  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

/**
 * Node Mass Update Batch 'finished' callback.
 */
function _job_mass_update_batch_finished($success, $results, $operations) {
  if ($success) {
    drupal_set_message(t('The update has been performed.'));
  }
  else {
    drupal_set_message(t('An error occurred and processing did not complete.'), 'error');
    $message = format_plural(count($results), '1 item successfully processed:', '@count items successfully processed:');
    $message .= theme('item_list', array('items' => $results));
    drupal_set_message($message);
  }
}

function jobbag_list_form($limit = 50) {
  $form['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Update options'),
    '#attributes' => array('class' => array('container-inline')),
  );
  $options = array();
  foreach (module_invoke_all('job_operations') as $operation => $array) {
    $options[$operation] = $array['label'];
  }

  $form['options']['operation'] = array(
    '#type' => 'select',
    '#title' => t('Operation'),
    '#title_display' => 'invisible',
    '#options' => $options,
    '#default_value' => 'approve',
  );

  $form['options']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
    '#validate' => array('job_admin_nodes_validate'),
    '#submit' => array('job_admin_nodes_submit'),
  );

  $header = array(
    'job_number' => array('data' => t('Job Number'), 'field' => 'j.title'),
    'type' => array('data' => t('Type'), 'field' => 'j.type'),
    'author' => t('Author'),
    'status' => array('data' => t('Status'), 'field' => 'j.status'),
    'changed' => array('data' => t('Updated'), 'field' => 'j.changed', 'sort' => 'desc'),
    'operations' => array('data' => t('Operations'))
  );

  $query = db_select('jobbag', 'j')
    ->extend('PagerDefault')
    ->extend('TableSort');

  job_filter_form_query($query);

  $jids = $query->fields('j', array('jid'))
    ->limit($limit)
    ->orderByHeader($header)
    ->execute()
    ->fetchCol();
  $jobs = jobbag_load_multiple($jids, array(), TRUE);

  $options = array();
  $destination = drupal_get_destination();
  foreach ($jobs as $job) {
    if (!jobbag_access('view', $job)) {
        continue;
    }

    $options[$job->jid] = array(
      'job_number' => array(
        'data' => array(
          '#type' => 'link',
          '#title' => $job->title,
          '#href' => 'job/' . $job->jid,
          '#suffix' => ' ' . theme('mark', array('type' => jobbag_mark($job->jid, $job->changed))),
        )
      ),
      'type' => check_plain(jobbag_get_type_name($job)),
      'author' => theme('username', array('account' => $job->user())),
      'status' => job_status((!empty($job->status) ? $job->status : 0)),
      'changed' => format_date($job->changed, 'short'),
    );

    // Build a list of all the accessible operations for the current node.
    $operations = array();

    if (jobbag_access('edit', $job)) {
      $operations['edit'] = array(
        'title' => t('edit'),
        'href' => 'job/' . $job->jid . '/edit',
        'query' => $destination,
      );
    }
    if (jobbag_access('delete', $job)) {
      $operations['delete'] = array(
        'title' => t('delete'),
        'href' => 'job/' . $job->jid . '/delete',
        'query' => $destination,
      );
    }

    $operations += module_invoke_all('job_list_operations', $job, $destination);

    $options[$job->jid]['operations'] = array();
      if (count($operations) > 1) {
        // Render an unordered list of operations links.
        $options[$job->jid]['operations'] = array(
        'data' => array(
          '#theme' => 'links',
          '#links' => $operations,
          '#attributes' => array('class' => array('links', 'inline')),
        ),
      );
    }
    elseif (!empty($operations)) {
      // Render the first and only operation as a link.
      $link = reset($operations);
      $options[$job->jid]['operations'] = array(
        'data' => array(
          '#type' => 'link',
          '#title' => $link['title'],
          '#href' => $link['href'],
          '#options' => array('query' => $link['query']),
        )
      );
    }
  }
	
  $form['jobs'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#empty' => t('No content available.'),
  );
	
  $form['pager'] = array('#markup' => theme('pager'));

  return $form;
}

function job_admin_nodes_validate($form, &$form_state) {
  // Error if there are no items to select.
  if (!is_array($form_state['values']['jobs']) || !count(array_filter($form_state['values']['jobs']))) {
    form_set_error('', t('No items selected.'));
  }
}

function job_admin_nodes_submit($form, &$form_state) {
  $operations = module_invoke_all('job_operations');
  $operation = $operations[$form_state['values']['operation']];
  // Filter out unchecked nodes
  $jobs = array_filter($form_state['values']['jobs']);
  if ($function = $operation['callback']) {
    // Add in callback arguments if present.
    if (isset($operation['callback arguments'])) {
      $args = array_merge(array($jobs), $operation['callback arguments']);
    }
    else {
      $args = array($jobs);
    }
    call_user_func_array($function, $args);

    cache_clear_all();
  }
  else {
    // We need to rebuild the form to go to a second step. For example, to
    // show the confirmation form for the deletion of nodes.
    $form_state['rebuild'] = TRUE;
  }
}

function job_multiple_delete_confirm($form, &$form_state, $jobs) {
  $form['jobs'] = array('#prefix' => '<ul>', '#suffix' => '</ul>', '#tree' => TRUE);
  // array_filter returns only elements with TRUE values
  foreach ($jobs as $jid => $value) {
    $title = db_select('job', 'j')->fields('j', array('title'))->condition('jid', $jid)->execute()->fetchField();
    $form['jobs'][$jid] = array(
      '#type' => 'hidden',
      '#value' => $jid,
      '#prefix' => '<li>',
      '#suffix' => check_plain($title) . "</li>\n",
    );
  }
  $form['operation'] = array('#type' => 'hidden', '#value' => 'delete');
  $form['#submit'][] = 'job_multiple_delete_confirm_submit';
  $confirm_question = t('Are you sure you want to delete @jobs?',
    array('@jobs' => format_plural(count($jobs), 'this job', 'these jobs'))
  );
  return confirm_form($form,
                    $confirm_question,
                    'admin/jobs', t('This action cannot be undone.'),
                    t('Delete'), t('Cancel'));
}

function job_multiple_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    entity_delete_multiple('job', array_keys($form_state['values']['jobs']));
    $count = count($form_state['values']['jobs']);
    
    drupal_set_message(format_plural($count, 'Deleted 1 job.', 'Deleted @count jobs.'));
  }
  $form_state['redirect'] = 'admin/jobs';
}

function jobbag_mark($jid, $changed) {
  $job = jobbag_load($jid);

  if ($job->created == $job->changed && $job->changed == $changed && (REQUEST_TIME - $changed) < (60*60*24)) {
    return MARK_NEW;
  }

  if ($job->changed == $changed && (REQUEST_TIME - $changed) < (60*60*24)) {
    return MARK_UPDATED;
  }

  return MARK_READ;
}

/*
 * Admin Job Type Area
 *
 */
function job_type_operation_form($form, &$form_state, $type_name, $job_type, $op) {
  if ($op == 'delete') {
    $form = array();
    return confirm_form($form, t('Are you sure you want to delete %type?',
      array('%type' => $job_type->type)), 'admin/jobs/manage');
  }

  return FALSE;
}

function job_type_form($form, &$form_state, $job_type, $op = 'edit') {
  if ($op == 'clone') {
    $job_type->label .= ' (cloned)';
    $job_type->type = '';
  }

  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $job_type->label,
    '#description' => t('The human-readable name of this profile type.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  // Machine-readable type name.
  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($job_type->type) ? $job_type->type : '',
    '#maxlength' => 32,
    '#disabled' => $job_type->isLocked() && $op != 'clone',
    '#machine_name' => array(
      'exists' => 'jobbag_get_types',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this job type. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save profile type'),
    '#weight' => 40,
  );

  if (!$job_type->isLocked() && $op != 'add' && $op != 'clone') {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete profile type'),
      '#weight' => 45,
      '#limit_validation_errors' => array(),
      '#submit' => array('job_type_form_submit_delete')
    );
  }
  return $form;
}

/**
 * Form API submit callback for the type form.
 */
function job_type_form_submit(&$form, &$form_state) {
  $job_type = entity_ui_form_submit_build_entity($form, $form_state);
  // Save and go back.
  entity_save($job_type->entityType(), $job_type);
  $form_state['redirect'] = 'admin/structure/jobs';
}

/**
 * Form API submit callback for the delete button.
 */
function job_type_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/jobs/manage/' . $form_state['job_type']->type . '/delete';
}


/*
 * Admin Job Client Area
 *
 */

function job_client_operation_form($form, &$form_state, $type_name, $job_client, $op) {
  if ($op == 'delete') {
    $form = array();
    return confirm_form($form, t('Are you sure you want to delete %type?',
      array('%type' => $job_client->label)), 'admin/jobs/clients');
  }

  return FALSE;
}
 
 function job_client_form($form, &$form_state, $client, $op = 'edit') {
  if ($op == 'clone') {
    $client->label .= ' (clone)';
    $client->prefix = '';
    $client->sequence = '001';
  }

  if ($op == 'edit') {
    $client = array_shift($client);
  }

  $form['#client'] = $client;

  $form_state['entity_type'] = 'job_client';

  $form['label'] = array(
    '#title' => t('Client Name'),
    '#type' => 'textfield',
    '#default_value' => $client->label,
    '#required' => TRUE,
    '#size' => 30,
  );

  $form['prefix'] = array(
    '#title' => t('Prefix'),
    '#type' => 'textfield',
    '#default_value' => $client->prefix,
    '#required' => TRUE,
    '#size' => 5,
    '#maxlength' => 5,
    '#description' => t('A set of letters that act as an identified for the client in the job number
      of any jobs associated with this client. IE: ABC-001')
  );

  $form['sequence'] = array(
    '#title' => t('Next Job Number in Sequence'),
    '#type' => 'textfield',
    '#default_value' => isset($client->sequence) ? str_pad($client->sequence, 3, '0', STR_PAD_LEFT) : '000',
    '#required' => TRUE,
    '#size' => 3,
    '#maxlength' => 3
  );

  $form['status'] = array(
    '#type' => 'radios',
    '#title' => t('Status'),
    '#options' => array(
      t('Disabled'),
      t('Active')
    ),
    '#default_value' => isset($client->status) ? $client->status : 1
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save client'),
    '#weight' => 40,
  );

  if ($op != 'add' && $op != 'clone') {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete client'),
      '#weight' => 45,
      '#limit_validation_errors' => array(),
      '#submit' => array('job_client_form_submit_delete')
    );
  }
  return $form;
}

function job_client_form_validate(&$form, &$form_state) {
  if (preg_match('/[^A-Za-z]+/', $form_state['values']['prefix'])) {
    form_error($form['prefix'], 'The Prefix contains illegal characters');
  }
  else {
    $prefix = db_select('jobbag_client', 'jc')->fields('jc', array('prefix', 'cid'))
      ->condition('prefix', $form_state['values']['prefix'])->execute()->fetchAllAssoc('cid');
    if ($client = $form['#client']) {
      unset($prefix[$client->cid]);
    }
    if (!empty($prefix)) {
      form_error($form['prefix'], 'The Prefix is already in use');
    }
    else {
      $form_state['values']['prefix'] = strtoupper($form_state['values']['prefix']);
    }
  }

  if (preg_match('/[^0-9]+/', $form_state['values']['sequence'])) {
      form_error($form['sequence'], 'The Prefix contains illegal characters');
  }
}

/**
 * Form API submit callback for the type form.
 */
function job_client_form_submit(&$form, &$form_state) {
  $client = $form['#client'];
  if (!$client) {
	$client = entity_ui_form_submit_build_entity($form, $form_state);
  }
  else {
	entity_form_submit_build_entity('job_client', $client, $form, $form_state);
  }
  if (is_array($client)) {
	$client = array_shift($client);
  }
  // Save and go back.
  entity_save($client->entityType(), $client);
  $form_state['redirect'] = 'admin/jobs/clients';
}

/**
 * Form API submit callback for the delete button.
 */
function job_client_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/jobs/clients/' . $form_state['client']->cid . '/delete';
}