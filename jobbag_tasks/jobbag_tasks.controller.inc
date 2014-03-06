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
}

class JobBagTaskTypeUIController extends EntityDefaultUIController {
  public function __construct($entity_type = NULL, $entity_info) {
    return parent::__construct('jobbag_task_type', $entity_info);
  }
}

class JobBagTaskController extends EntityAPIControllerExportable {
  public function create(array $values = array()) {
    return parent::create($values, 'jobbag_task');
  }
}

class JobBagTaskStatusTypeController extends EntityAPIControllerExportable {
  public function create(array $values = array()) {
    return parent::create($values, 'jobbag_task_status_type');
  }
}

class JobBagTaskStatusController extends EntityAPIControllerExportable {
  public function create(array $values = array()) {
    return parent::create($values, 'jobbag_task_status');
  }
}

class JobBagTaskStatusUIController extends EntityDefaultUIController {
  public function __construct($entity_type = NULL, $entity_info) {
    return parent::__construct('jobbag_task_status', $entity_info);
  }
}

class JobBagTaskWorkflowTypeController extends EntityAPIControllerExportable {
  public function create(array $values = array()) {
    return parent::create($values, 'jobbag_task_workflow_type');
  }
}

class JobBagTaskWorkflowController extends EntityAPIControllerExportable {
  public function create(array $values = array()) {
    return parent::create($values, 'jobbag_task_workflow');
  }
}

class JobBagTaskWorkflowUIController extends EntityDefaultUIController {
  public function __construct($entity_type = NULL, $entity_info) {
    return parent::__construct('jobbag_task_workflow', $entity_info);
  }
}

class JobBagStatusWorkflowTypeController extends EntityAPIControllerExportable {
  public function create(array $values = array()) {
    return parent::create($values, 'jobbag_status_workflow_type');
  }
}

class JobBagStatusWorkflowController extends EntityAPIControllerExportable {
  public function create(array $values = array()) {
    return parent::create($values, 'jobbag_status_workflow');
  }

  public function currentStatus(JobBagStatusWorkflow $workflow) {

  }

  public function getStatuses(JobBagStatusWorkflow $workflow) {

  }
}

class JobBagStatusWorkflowUIController extends EntityDefaultUIController {
  public function __construct($entity_type = NULL, $entity_info) {
    return parent::__construct('jobbag_status_workflow', $entity_info);
  }
}