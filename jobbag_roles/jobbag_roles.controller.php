<?php

class JobBagRoleControllerExportable extends EntityAPIControllerExportable {
  public function export($entity, $prefix = '') {
    if (is_array($entity)) {
      $entity = array_shift($entity);
    }
    return parent::export($entity, $prefix);
  }

  public function save($entity) {
    $entity->changed = REQUEST_TIME;
    if (isset($entity->is_new) || !isset($entity->created)) {
      $entity->created = REQUEST_TIME;
    }
    if (!isset($entity->module)) {
      $entity->module = 'jobbag_roles';
    }
    return parent::save($entity);
  }

  public function hasPermission(JobBagRole $role, $op) {
    if (!isset($role->permissions) || !is_array($role->permissions)) {
      return FALSE;
    }
    return in_array($op, $role->permissions);
  }
}

class JobBagRoleUIController extends EntityDefaultUIController {
  public function hook_menu() {
    $items = parent::hook_menu();
    $items[$this->path]['type']['title'] = t('Job Roles');
    $items[$this->path]['type'] = MENU_LOCAL_ACTION;
    $items[$this->path]['tab_parent'] = 'admin/jobs';
    $items[$this->path]['tab_root'] = 'admin/jobs';
    $items[$this->path]['context'] = MENU_CONTEXT_PAGE | MENU_CONTEXT_INLINE;

    return $items;
  }

  public function overviewForm($form, &$form_state) {
    drupal_set_title('Job Roles');
    return parent::overviewForm($form, $form_state);
  }
}

class JobRoleController extends EntityAPIController {
  public function create(array $values = array()) {
    $entity = parent::create($values, 'job_role');
    if (!isset($entity->role)) {
      $entity->role = jobbag_role_load($entity->rid);
    }
    return $entity;
  }

  public function buildQuery($ids, $conditions = array(), $revision_id = FALSE) {
    $query = parent::buildQuery($ids, $conditions, $revision_id);
    $query->groupBy('rid');
    return $query;
  }

  public function load($ids = array(), $conditions = array()) {
    $entities = parent::load($ids, $conditions);
    foreach ($entities as $entity) {;
      $entity->role = jobbag_role_load($entity->rid);
      $entity->setUsers($entity->users);
    }
    return $entities;
  }

  public function loadByJob(JobBag $job, $conditions = array()) {
    $job_info = $job->entityInfo();
    if (array_key_exists($job_info['entity keys']['id'], $conditions)) {
      unset($conditions[$job_info['entity keys']['id']]);
    }

    $query = db_select($this->entityInfo['base table'], 'bt')
      ->fields('bt', array('jrid'))
      ->condition('jid', $job->identifier());

    foreach ($conditions as $field => $value) {
      $query->condition($field, $value);
    }

    $jrids = $query->execute()->fetchAllAssoc('jrid');
    $ids = array_keys($jrids);
    return !empty($ids) ? $this->load($ids) : FALSE;
  }

  public function access(JobRole $role, $op, $account = NULL) {
    global $user;
    if (!isset($account)) {
      $account = $user;
    }

    if (!empty($role->users) && !empty($role->permissions) && in_array($account->uid, $role->users)) {
      return in_array('full access', $role->permissions) || in_array($op, $role->permissions);
    }

    return NULL;
  }

  public function hasPermission(JobRole $role, $op) {
    if (!is_array($role->permissions)) {
      return FALSE;
    }
    return in_array($op, $role->permissions);
  }

  public function setUsers(JobRole $role, $uids = array()) {
    if (!empty($uids)) {
      $role->users = reset($uids) && is_numeric(current($uids)) ?  user_load_multiple($uids) : $uids;
    }
  }

  public function addUser(JobRole $role, stdClass $account) {
    if (!$role->hasUser($account)) {
      $role->users[$account->uid] = $account;
    }
    return $role;
  }

  public function getUsers(JobRole $role) {
    return $role->users;
  }

  public function hasUser(JobRole $role, stdClass $account) {
    return isset($role->users[$account->uid]) && $role->users[$account->uid] === $account;
  }

  public function save($role) {
    $users = $role->users;
    $uids = array_keys($users);
    $role->users = $uids;
    $success = parent::save($role);
    $role->users = $users;
    return $success;
  }

  public function invoke($hook, $role) {
    $args = func_get_args();
    array_shift($args); // Shift off hook

    if ($hook == 'user_added' || $hook == 'user_deleted') {
      array_unshift($args, $this->entityType().'_'.$hook);
      call_user_func_array('rules_invoke_event', $args);
    }
    else {
      parent::invoke($hook, $role);
    }
  }
}