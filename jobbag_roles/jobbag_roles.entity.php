<?php

class JobRole extends Entity {
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
    
    public function save() {
        if (empty($this->created) && (!empty($this->is_new) || !$this->jid))
            $this->created = REQUEST_TIME;
        $this->changed = REQUEST_TIME;
    
        parent::save();
    }
}