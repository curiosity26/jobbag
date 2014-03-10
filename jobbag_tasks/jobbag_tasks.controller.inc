<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 2/22/14
 * Time: 10:31 AM
 */

class JobBagTaskTypeController extends EntityAPIControllerExportable {
  public function create(array $values = array()) {
    return parent::create($values, 'jobbag_task_type');
  }

  public function save($entity) {
    $entity->changed = REQUEST_TIME;
    if (isset($entity->is_new)) {
      $entity->created = REQUEST_TIME;
    }

    if (!isset($entity->module)) {
      $entity->module = 'jobbag_tasks';
    }

    return parent::save($entity);
  }
}

class JobBagTaskTypeUIController extends EntityDefaultUIController {
  public function __construct($entity_type = NULL, $entity_info) {
    return parent::__construct('jobbag_task_type', $entity_info);
  }
}

class JobBagTaskController extends EntityAPIController {
  public function create(array $values = array()) {
    return parent::create($values, 'jobbag_task');
  }

  public function roles(JobBagTask $entity) {
    $ids = $entity->roles;
    if (!empty($ids)) {
      return entity_load('jobbag_role', $ids);
    }
    return FALSE;
  }

  public function accounts(JobBagTask $entity) {
    $ids = $entity->accounts;
    if (!empty($ids)) {
      return user_load_multiple($ids);
    }
    return FALSE;
  }

  public function job(JobBagTask $entity) {
    if ($entity->job) {
      return job_load($entity->job);
    }
    return FALSE;
  }

  public function buildContent($entity, $view_mode = 'full', $langcode = NULL) {
    $output = parent::buildContent($entity, $view_mode, $langcode);
    if ($view_mode == 'field') {
      $output['#type'] = 'task_field';
      $output['#entity'] = $entity;
    }
    return $output;
  }

  public function save($entity) {
    $entity->changed = REQUEST_TIME;
    if (isset($entity->is_new)) {
      $entity->created = REQUEST_TIME;
      $entity->status = 'new';
    }

    if (!isset($entity->module)) {
      $entity->module = 'jobbag_tasks';
    }

    return parent::save($entity);
  }
}