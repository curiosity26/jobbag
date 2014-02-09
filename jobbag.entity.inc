	<?php

class JobBag extends Entity {
  public $jid;
  public $type;
  public $uid;
  public $title;
  public $description;
  public $client_id;
  public $client;
  public $status;
  public $created;
  public $changed;
  public $closed;
  public $cancelled;
  public $data = array();

  public function __construct($values = array()) {
    if (isset($values['user'])) {
      $this->setUser($values['user']);
      unset($values['user']);
    }

    if (isset($values['type']) && is_object($values['type'])) {
      $values['type'] = $values['type']->type;
    }

    if (!isset($values['label']) && isset($values['type']) && ($type = jobbag_get_types($values['type']))) {
      $values['label'] = $type->label;
    }

    parent::__construct($values, 'job');

    if (!is_array($this->data)) {
      $this->data = array();
    }
  }

  public function defaultTitle() {
    return t('@client - @job_number',
      array('@client' => $this->getClient()->label(), '@job_number' => $this->getJobNumber()));
  }

  protected function defaultUri() {
    return array(
      'path' => 'job/' . $this->identifier()
    );
  }

  public function getJobNumber() {
    return $this->title;
  }

  public function getClient() {
    if (empty($this->client)) {
      $this->setClient($this->client_id);
    }

    return $this->client;
  }

  public function setClient($client) {
    if (is_numeric($client)) {
      $clients = entity_load('job_client', array($client));
      $client = array_shift($clients);
    }

    if (is_object($client) && !empty($client->cid)) {
      $this->client = $client;
    }

    return $this;
  }

  public function user() {
     return user_load($this->uid);
  }

  public function type() {
    return jobbag_get_types($this->type);
  }

  public function setUser($account) {
     $this->uid = is_object($account) ? $account->uid : $account;
  }

  public function path() {
    $uri = $this->uri();
    return $uri['path'];
  }

  public function url() {
    $uri = $this->uri();
    return url($uri['path'], $uri);
  }

  public function getReminder() {
    $controller = entity_get_controller($this->entityType());
    return $controller->getReminder($this);
  }

  public function getReminderUnit() {
    $controller = entity_get_controller($this->entityType());
    return $controller->getReminderUnit($this);
  }

  public function getReminderMetric() {
    $controller = entity_get_controller($this->entityType());
    return $controller->getReminderMetric($this);
  }

  public function setReminder($metric = NULL, $unit = NULL) {
    $controller = entity_get_controller($this->entityType());
    return $controller->setReminder($this, $metric, $unit);
  }

  public function setReminderTime($time) {
    $controller = entity_get_controller($this->entityType());
    return $controller->setReminderTime($this, $time);
  }
}

class JobBagType extends Entity {
  public $id;
  public $type;
  public $label;
  public $status;
  public $module;
  public $weight = 0;

  public function __construct($values = array()) {
    parent::__construct($values, 'job_type');
  }

  public function isLocked() {
    return isset($this->status) && empty($this->is_new) && (($this->status & ENTITY_IN_CODE) || ($this->status & ENTITY_FIXED));
  }
}

class JobBagClient extends Entity {
  public $cid;
  public $prefix;
  public $label;
  public $sequence;
  public $created;
  public $changed;
  public $status;

  public function __construct($values = array()) {
    parent::__construct($values, 'job_client');
  }

  protected function defaultUri() {
    return array(
      'path' => 'job/client/' . $this->identifier()
    );
  }

  public function path() {
    $uri = $this->uri();
    return $uri['path'];
  }

  public function getPrefix() {
    return $this->prefix;
  }

  public function setPrefix(string $p) {
    if (preg_match('/[^A-Za-z]+/', $p) !== FALSE) {
      $this->prefix = strtoupper($p);
    }

    return $this;
  }

  public function getLabel() {
    return $this->label;
  }

  public function setLabel(string $l) {
    $this->label = $l;
    return $this;
  }

  public function increaseSequence() {
    $this->sequence++;
    entity_save($this->entityType(), $this);
    return $this;
  }
}