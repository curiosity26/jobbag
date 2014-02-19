<?php

class JobBagRole extends Entity {
  public $rid;
  public $label;
  public $machine_name;
  public $filter_by;
  public $filter_users = array();
  public $default_users = array();
  public $permissions = array();
  public $module;
  public $cardinality;
  public $weight;
  public $created;
  public $changed;

  public function __construct($values = array()) {
      parent::__construct($values, 'jobbag_role');
  }

  public function hasPermission($op) {
    $controller = entity_get_controller($this->entityType());
    return $controller->hasPermission($this, $op);
  }

  public function save() {
    if (empty($this->created) && (isset($this->is_new) || !$this->jid)) {
      $this->created = REQUEST_TIME;
    }
    $this->changed = REQUEST_TIME;

    parent::save();
  }
}

class JobRole extends Entity {
  public $jrid;
  public $jid;
  public $rid;
  public $users = array();
  public $permissions = array();

  public function __construct(array $values = array()) {
    return parent::__construct($values, 'job_role');
  }

  public function access($op, $account = NULL) {
    $controller = entity_get_controller($this->entityType());
    return $controller->access($this, $op, $account);
  }

  public function hasPermission($op) {
    $controller = entity_get_controller($this->entityType());
    return $controller->hasPermission($this, $op);
  }

  public function setUsers(array $uids = array()) {
    $controller = entity_get_controller($this->entityType());
    return $controller->setUsers($this, $uids);
  }

  public function getUsers() {
    $controller = entity_get_controller($this->entityType());
    return $controller->getUsers($this);
  }

  public function addUser(stdClass $account) {
    $controller = entity_get_controller($this->entityType());
    return $controller->addUser($this, $account);
  }

  public function hasUser(stdClass $account) {
    $controller = entity_get_controller($this->entityType());
    return $controller->hasUser($this, $account);
  }

  public function __sleep() {
    $vars = parent::__sleep();
    unset($vars['role']);
    return $vars;
  }

  public function __wakeup() {
    parent::__wakeup();
    $this->role = jobbag_role_load($this->rid);
  }
}