<?php

function jobbag_rules_action_info() {

  $actions['job_notifications'] = array(
    'label' => t('Process job notifications settings'),
    'base' => 'rules_actions_job_notifications',
    'named parameter' => TRUE,
    'parameter' => array(
      'job' => array(
        'type' => 'entity',
        'label' => t('Job'),
      )
    ),
    'group' => t('Job'),
    'access callback' => 'rules_user_integration_access'
  );

  $actions['job_notifications_cleanup'] = array(
    'label' => t('Delete job notifications for deleted job'),
    'base' => 'rules_actions_job_notifications_cleanup',
    'named parameter' => TRUE,
    'parameter' => array(
      'job' => array(
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
      'job' => array(
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
  module_load_include('inc', 'jobbag', 'jobbag.defaults');
  $rules = array();

  $job_notify = rules_reaction_rule();
  $job_notify->label = t('Job is saved');
  $job_notify->event('job_insert');
  $job_notify->event('job_update');
  $job_notify->action('job_notifications', array('job' => 'job'));
  $rules['job_notification_setup'] = $job_notify;

  $closed = rules_reaction_rule();
  $closed->label = t('Job is closed out');
  $closed->event('job_insert');
  $closed->event('job_update');
  $closed->condition('data_is', array('data:select' => 'job:status', 'value' => JOB_STATUS_CLOSED));
  $closed->action('job_send_notification', array(
      'job' => 'job',
      'subject' => variable_get('job_cancelled_subject', _jobbag_default_cancelled_subject()),
      'message' => variable_get('job_cancelled_message', _jobbag_default_cancelled_message())
    )
  );
  $closed->action('job_notifications_cleanup', array(
    'job' => 'job'
  ));
  $rules['job_closed_send'] = $closed;

  $cancelled = rules_reaction_rule();
  $cancelled->label = t('Job is cancelled');
  $cancelled->event('job_insert');
  $cancelled->event('job_update');
  $cancelled->condition('data_is', array('data:select' => 'job:status', 'value' => JOB_STATUS_CANCELLED));
  $cancelled->action('job_send_notification', array(
      'job' => 'job',
      'subject' => variable_get('job_cancelled_subject', _jobbag_default_cancelled_subject()),
      'message' => variable_get('job_cancelled_message', _jobbag_default_cancelled_message())
    )
  );
  $cancelled->action('job_notifications_cleanup', array(
    'job' => 'job'
  ));
  $rules['job_cancelled_send'] = $cancelled;

  $cleanse = rules_reaction_rule();
  $cleanse->label = t('Job is deleted');
  $cleanse->event('job_delete');
  $cleanse->action('job_notifications_cleanup');
  $rules['job_deleted'] = $cleanse;

  return $rules;
}

/* Actions Callbacks */

function rules_actions_job_notifications($args, $element) {
  module_load_include('inc', 'jobbag', 'jobbag.defaults');
  $job = $args['state']->get('job')->value();
  $date = new DateObject($job->job_due_date['und'][0]['value'], $job->job_due_date['und'][0]['timezone']);
  $due = $date->getTimestamp();
  $notifications = array(
    'reminder' => $due - $job->reminder(),
    'date_due' => $due,
    'past_due' => $due + 86400,
  );


  foreach ($notifications as $notify => $time) {
    $component = rules_config_load('job_'.$job->identifier().'_'.$notify);
    if ((($job->data[$notify.'_override'] == FALSE
          && variable_get('job_'.$notify.'_disabled', FALSE) == FALSE)
        || ($job->data[$notify.'_override'] == TRUE
          && variable_get('job_'.$notify.'_disabled', FALSE) == TRUE))) {

      if(($reminder_send = rules_config_load('job_'.$job->identifier().'_'.$notify.'_send')) === FALSE) {
        $reminder_send = rule();
        $reminder_send->label = t('Send a @notify notification for @job',
          array('@job' => $job->title, '@notify' => $notify));
        $reminder_send->action('entity_fetch', array(
            'type' => 'job',
            'id' => $job->identifier()
          )
        );

        $reminder_send->action('job_send_notification', array(
          'job' => $job,
          'subject' => variable_get('job_'.$notify.'_subject', _jobbag_default_reminder_subject()),
          'message' => variable_get('job_'.$notify.'_message', _jobbag_default_reminder_message())
        ));

        $reminder_send->save('job_'.$job->identifier().'_'.$notify.'_send', 'jobbag');
      }

      $action = rules_action('schedule', array(
        'component' => 'job_'.$job->identifier().'_'.$notify.'_send',
        'date' => $time,
        'identifier' => 'job_'.$job->identifier().'_'.$notify
      ));
      $action->executeByArgs();
    }
    elseif ((($job->data[$notify.'_override'] == FALSE
          && variable_get('job_'.$notify.'_disabled', FALSE) == TRUE)
        || ($job->data[$notify.'_override'] == TRUE
          && variable_get('job_'.$notify.'_disabled', FALSE) == FALSE))
      && isset($component)) {
      rules_config_delete(array('job_'.$job->identifier().'_'.$notify.'_send',
        'job_'.$job->identifier().'_'.$notify));
    }
  }
}

function rules_actions_job_send_notification($args, $element) {
  $job = $args['state']->get('job')->value();
  $subject = $args['state']->get('subject')->value();
  $message = $args['state']->get('message')->value();
  $user = $job->user();

  $action = rules_action('mail', array(
    'to' => $user->mail,
    'subject' => $subject,
    'message' => $message,
    'language' => $user->language
  ));

  drupal_alter('job_send_notifcation', $action, $args, $element);

  $action->executeByArgs();
}

function rules_actions_job_notifications_cleanup($args, $element) {
  $job = $args['state']->get('job')->value();
  $notifications = array(
    'reminder',
    'date_due',
    'past_due'
  );
  foreach ($notifications as $notify) {
    $comps = array('job_'.$job->identifier().'_'.$notify,
      'job_'.$job->identifier().'_'.$notify.'_send');
    rules_config_delete($comps);
  }
}