<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 2/22/14
 * Time: 10:31 AM
 */

class JobBagTaskType extends Entity {
  public function __construct($values = array()) {
    return parent::__construct($values, 'jobbag_task_type');
  }
}

class JobBagTask extends Entity {
  public $roles = array();
  public $accounts = array();
  public $job;
  public $status = 'new';

  public function __construct($values = array()) {
    return parent::__construct($values,'jobbag_task');
  }

  public function roles() {
    $controller = $this->entityInfo['controller class'];
    return $controller->roles($this);
  }

  public function accounts() {
    $controller = $this->entityInfo['controller class'];
    return $controller->accounts($this);
  }

  public function job() {
    $controller = $this->entityInfo['controller class'];
    return $controller->job($this);
  }
}