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
      parent::__construct($values, 'job_roles');
  }

  public function hasPermission($op) {
    dpm($this, 'JobBagRole');
    $class = $this->entityInfo['controller class'];
    return $class::hasPermission($this, $op);
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
  public $permissons = array();

  public function access($op, $account = NULL) {
    $class = $this->entityInfo['controller class'];
    return $class::access($this, $op, $account);
  }

  public function hasPermission($op) {
    $class = $this->entityInfo['controller class'];
    return $class::hasPermission($this, $op);
  }

  public function setUsers(array $uids = array()) {
    $class = $this->entityInfo['controller class'];
    return $class::setUsers($this, $uids);
  }

  public function getUsers() {
    $class = $this->entityInfo['controller class'];
    return $class::getUsers($this);
  }

  public function addUser(stdClass $account) {
    $class = $this->entityInfo['controller class'];
    return $class::addUser($this, $account);
  }

  public function hasUser(stdClass $account) {
    $class = $this->entityInfo['controller class'];
    return $class::hasUser($this, $account);
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