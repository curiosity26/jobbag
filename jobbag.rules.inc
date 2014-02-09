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
  // TODO: define any custom actions to be handled on the job here
	$defaults = array(
      'parameter' => array(
        'job' => array(
            'type' => 'entity',
            'label' => t('Job'),
            'default mode' => 'selector'
          ),
        'subject' => array(
            'type' => 'text',
            'label' => t('Subject'),
            'restriction' => 'input'
          ),
        'message' => array(
            'type' => 'text',
            'label' => t('Email message'),
            'restriction' => 'input'
          )
        ),
      'group' => t('Job'),
      'access callback' => 'rules_user_integration_access'
    );

	$actions['job_send_email'] = $defaults + array(
      'label' => t('Send a job alert email'),
      'base' => 'rules_action_job_send_email'
    );

	return $actions;
}

function jobbag_default_rules_configuration() {
  // TODO: define default rules
  // TODO: define rule for scheduling when new job is created
  // TODO: define notification rule for date due reminder
  // TODO: define notification rule for due date arrival
  // TODO: define notification rule for due date past, recreating itself until removed by the job cancelled/completed/deleted rule
  // TODO: define rule for removing rules in the sytem when cancelled, completed, or deleted

  $rules = array();

  $rule = rules_reaction_rule(array(
      'jobs' => array('type' => 'list<entity>', 'label' => 'Jobs')
    )
  );

  $rule->label = t('Check active jobs to see if their due date is approaching');
  $rule->event('job_cron_check_jobs');
  $rule->active = TRUE;

  $loop_rule = rule();
  $loop_rule->label = t('Check if jobs are approaching their due date');
  $loop_rule->condition('job_check_due_date', array(
      'job' => 'job',
      'days' => 2
    )
  );

  $loop_rule->action('job_send_email', array(
      'job' => 'job',
      'subject' => '[job:job_number] is approaching its due date',
      'message' => 'Job number [job:job_number] is due to be complete as of [job:job_due_date].
          Details about this job can be found at [job:url].

          Thank You'
    )
  );

  $loop = rules_loop(array(
      'list:select' => 'jobs',
      'item:var' => 'job',
      'item:label' => t('Job'),
      'item:type' => 'job'
    )
  )->action($loop_rule);

  $rule->action($loop);

  $rules['jobbag_check_due_date'] = $rule;

  return $rules;
}

function rules_condition_job_check_due_date($job, $days = 0) {
	if (is_numeric($job) || is_string($job))
		$job = jobbag_load((int) $job);
	if (!$job->job_due_date && $job->jid)
		$job = jobbag_load($job->jid);
	$dateraw = field_get_items('job', $job, 'job_due_date');
	if (!empty($dateraw)) {
		$date = field_view_value('job', $job, 'job_due_date', $dateraw, 'full');
		return strtotime("+$days Days") == strtotime($date['#markup']);
	}
	
	return FALSE;
}


function rules_action_job_send_email($job, $subject, $message) {
	$user = user_load(array('uid' => $job->uid));
	drupal_mail('jobbag', 'job_check_due_date', $user->mail, $job->language, 
		array('job' => $job, 'subject' => $subject, 'message' => $message));
}