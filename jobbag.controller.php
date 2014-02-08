<?php

class JobEntityController extends EntityAPIController {
  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    $content['#theme'] = 'jobbag';
    return parent::buildContent($entity, $view_mode, $langcode, $content);
  }

  public function save($entity) {
    $actions = array();
    if (empty($entity->created) && (!empty($entity->is_new) || !$entity->identifier())) {
      $entity->created = REQUEST_TIME;
      $entity->setClient($entity->client_id);
      $entity->title = $this->getClient()->prefix.'-'.str_pad($entity->getClient()->sequence, 3, '0', STR_PAD_LEFT);
      $entity->client->increaseSequence();

      $actions = trigger_get_assigned_actions('job_created');
    }

    $entity->changed = REQUEST_TIME;

    $saved_actions = trigger_get_assigned_actions('job_saved');
    $actions = array_merge($actions, $saved_actions);
    if ($actions) {
      actions_do($actions, $entity);
    }
    parent::save($entity);
  }
}

class JobClientUIController extends EntityDefaultUIController {
  public function hook_menu() {
    $items = parent::hook_menu();
    $items[$this->path]['title'] = t('Clients');
    $items[$this->path]['type'] = MENU_LOCAL_TASK;

    return $items;
  }
}

class JobClientControllerExportable extends EntityAPIControllerExportable {
  public function export($entity, $prefix = '') {
    if (is_array($entity)) {
      $entity = array_shift($entity);
    }
    return parent::export($entity, $prefix);
  }
}

class JobTypeControllerExportable extends EntityAPIControllerExportable {
  public function export($entity, $prefix = '') {
    if (is_array($entity)) {
      $entity = array_shift($entity);
    }

    return parent::export($entity, $prefix);
  }
	
  public function save($entity) {
    $entity->changed = REQUEST_TIME;

    if (empty($entity->created) && (!empty($entity->is_new) || !$entity->cid)) {
      $entity->created = $entity->changed;
    }

    $entity->prefix = strtoupper($entity->prefix);

    if (empty($this->module)) {
      $entity->module = 'jobbag';
    }

    parent::save($entity);

    $client_instance = field_read_instance('job', 'job_client', $entity->type);
    if (!$client_instance) {
      $client_instance = array(
        'field_name' => 'job_client_id',
        'entity_type' => 'job',
        'bundle' => 'default',
        'label' => t('Client'),
        'settings' => array(),
        'widget' => array('type' => 'options_select', 'weight' => -10),
        'display' => array(
          'default' => array(
            'type' => 'entityreference_label',
            'settings' => array(
              'link' => 1
            ),
            'weight' => 1
          ),
          'teaser' => array(
            'type' => 'entityreference_label',
            'settings' => array(
              'link' => 0
            ),
            'weight' => 1
          )
        )
      );

      field_create_instance($client_instance);
    }

    $job_ref_instance = field_read_instance('job', 'job_reference', $entity->type);
    if (!$job_ref_instance) {
      $job_ref_instance = array(
        'field_name' => 'job_reference',
        'entity_type' => 'job',
        'bundle' => 'default',
        'label' => t('Job Reference'),
        'settings' => array(),
        'widget' => array('type' => 'entityreference_autocomplete_tags', 'weight' => -9),
        'display' => array(
          'default' => array(
            'type' => 'entityreference_label',
            'settings' => array(
              'link' => 1
            ),
            'weight' => 2
          ),
          'teaser' => array(
            'type' => 'hidden',
            'weight' => 2
          )
        )
      );
      field_create_instance($job_ref_instance);
    }

    $date_instance = field_read_instance('job', 'job_due_date', $entity->type);
    if (!$date_instance) {
      $date_instance = array(
        'field_name' => 'job_due_date',
        'entity_type' => 'job',
        'bundle' => 'default',
        'label' => t('Date Due'),
        'settings' => array(),
        'widget' => array('type' => module_exists('date_popup') ? 'date_popup' : 'date_text', 'weight' => -8),
        'display' => array(
          'default' => array(
            'weight' => 3
          ),
          'teaser' => array(
            'type' => 'hidden',
            'weight' => 3
          )
        ),
      );
      field_create_instance($date_instance);
    }

    $desc_instance = field_read_instance('job', 'job_description', $entity->type);
    if (!$desc_instance) {
      $desc_instance = array(
        'field_name' => 'job_description',
        'entity_type' => 'job',
        'bundle' => 'default',
        'label' => t('Description'),
        'settings' => array(),
        'widget' => array('type' => 'text_textarea_with_summary', 'weight' => -7),
        'display' => array(
          'default' => array(
            'type' => 'text_default',
            'weight' => 4
          ),
          'teaser' => array(
            'type' => 'text_summary_or_trimmed',
            'settings' => array(
              'trim_length' => 600
            ),
            'weight' => 4
          )
        )
      );
      field_create_instance($desc_instance);
    }
  }
}