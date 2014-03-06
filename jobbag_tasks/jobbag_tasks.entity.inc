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
  public function __construct($values = array()) {
    return parent::__construct($values,'jobbag_task');
  }
}

class JobBagTaskStatusType extends Entity {
  public function __construct($values = array()) {
    return parent::__construct($values, 'jobbag_task_status_type');
  }
}

class JobBagTaskStatus extends Entity {
  public function __construct($values = array()) {
    return parent::__construct($values, 'jobbag_task_status');
  }
}

class JobBagTaskWorkflowType extends Entity {
  public function __construct($values = array()) {
    return parent::__construct($values, 'jobbag_task_workflow_type');
  }
}

class JobBagTaskWorkflow extends Entity {
  public function __construct($values = array()) {
    return parent::__construct($values, 'jobbag_task_workflow');
  }
}

class JobBagStatusWorkflowType extends Entity {
  public function __construct($values = array()) {
    return parent::__construct($values, 'jobbag_status_workflow_type');
  }
}

class JobBagStatusWorkflow extends Entity {
  public function __construct($values = array()) {
    return parent::__construct($values, 'jobbag_status_workflow');
  }
}