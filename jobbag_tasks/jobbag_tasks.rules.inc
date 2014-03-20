<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 3/19/14
 * Time: 7:47 PM
 */

function jobbag_tasks_rules_event_info() {
  $info = array();

  $info['jobbag_task_delegate'] = array(
    'label' => t('Task is delegated'),
    'type' => 'jobbag_task'
  );

  $info['jobbag_task_accept'] = array(
    'label' => t('Task is accepted'),
    'type' => 'jobbag_task'
  );

  $info['jobbag_task_reject'] = array(
    'label' => t('Task is rejected'),
    'type' => 'jobbag_task'
  );

  $info['jobbag_task_complete'] = array(
    'label' => t('Task is completed'),
    'type' => 'jobbag_task'
  );

  return $info;
}