<?php

function jobbag_rules_event_info() {
	$events = array();
	// TODO: Create any special event definitions

	return $events;
}

function jobbag_rules_condition_info() {
  $defaults = array(
    'group' => t('Job'),
    'access callback' => 'rules_user_integration_access'
  );

  $conditions = array();

  // TODO:  Due dates will be configurable based on job and held in the job entity, create any special conditions here

  return $conditions;
}

function jobbag_rules_action_info() {

  $actions['job_notifications'] = array(
    'label' => t('Process job notifications settings'),
    'base' => 'rules_actions_job_notifications',
    'named parameter' => TRUE,
    'parameter' => array(
      'entity' => array(
        'type' => 'entity',
        'label' => t('Job'),
      )
    ),
    'group' => t('Job'),
    'access callback' => 'rules_user_integration_access'
  );

  $actions['job_send_notification'] = array(
    'label' => t('Send a tokenized notification email about a job'),
    'base' => 'rules_actions_job_send_notification',
    'name parameter' => TRUE,
    'parameter' => array(
      'entity' => array(
        'type' => 'entity',
        'label' => t('Job')
      ),
      'subject' => array(
        'type' => 'text',
        'label' => t('Subject')
      ),
      'message' => array(
        'type' => 'text',
        'label' => t('Message')
      )
    ),
    'group' => t('Job'),
    'access callback' => 'rules_user_integration_access'
  );

  return $actions;
}

function jobbag_default_rules_configuration() {
  $rules = array();

  $job_notify = rules_reaction_rule();
  $job_notify->event('job_save');
  $job_notify->action('job_notifications');
  $rules['job_notification_setup'] = $job_notify;

  $closed = rules_reaction_rule();
  $closed->event('job_closed');
  $closed->action('job_send_notification', array(
      'entity' => $closed->availableVariables()['entity'],
      'subject' => variable_get('job_closed_subject', _jobbag_default_closed_subject()),
      'message' => variable_get('job_closed_message', _jobbag_default_closed_message())
    )
  );
  $rules['job_closed_send'] = $closed;

  $cancelled = rules_reaction_rule();
  $cancelled->event('job_cancelled');
  $cancelled->action('job_send_notification', array(
      'entity' => $cancelled->availableVariables()['entity'],
      'subject' => variable_get('job_cancelled_subject', _jobbag_default_cancelled_subject()),
      'message' => variable_get('job_cancelled_message', _jobbag_default_cancelled_message())
    )
  );
  $rules['job_cancelled_send'] = $cancelled;

  return $rules;
}

/* Actions Callbacks */

function rules_actions_job_notifications($args, $element) {
  module_load_include('inc', 'jobbag', 'jobbag.defaults');

  $job = $args['entity'];
  $user = user_load($job->uid);

  $notifications = array(
    'reminder' => $job->getReminder(),
    'date_due' => strtotime($job->job_due_date),
    'past_due' => strtotime($job->job_due_date) + 86400,
  );

  foreach ($notifications as $notify => $time) {
    $component = rules_config_load('job_'.$job->identifier().'_'.$notify);
    if ($job->data[$notify.'_enabled'] == TRUE) {
      $schedule_settings = array(
        'job_'.$job->identifier().'_'.$notify.'_send',
        $time,
        'job_'.$job->identifier().'_'.$notify
      );

      $reminder_send = rule();
      $reminder_send->action('entity_fetch', array(
          'type' => 'job',
          'id' => $job->identifier()
        )
      );

      $reminder_send->action('job_send_notification', array(
        'entity' => $args['state']['entity'],
        'subject' => variable_get('job_'.$notify.'_subject', _jobbag_default_reminder_subject()),
        'message' => variable_get('job_'.$notify.'_message', _jobbag_default_reminder_message()),
        'langcode' => $user->language
      ));

      $reminder_send->save('job_'.$job->identifier().'_'.$notify.'_send', 'jobbag');

      $schedule_rule = rule();
      $schedule_rule->label = t('Set a @notify notification for @job',
        array('@job' => $job->title, '@notify' => $notify));
      $schedule_rule->action('schedule', $schedule_settings);
      $schedule_rule->save('job_'.$job->identifier().'_'.$notify, 'jobbag');
    }
    elseif ($component) {
      rules_config_delete(array('job_'.$job->identifier().'_'.$notify, 'job_'.$job->identifier().'_'.$notify.'_send'));
    }
  }
}

function rules_actions_job_send_notification($args, $element) {
  $job = $args['entity'];
  $user = user_load($job->uid);
  $subject = token_replace($args['subject'], array('job' => $job));
  $message = token_replace($args['message'], array('job' => $job));

  $action = rules_action('mail', array(
    'to' => $user->mail,
    'subject' => $subject,
    'message' => $message
  ));

  $action->execute();
}