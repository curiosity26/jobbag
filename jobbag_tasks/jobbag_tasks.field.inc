<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 2/23/14
 * Time: 11:07 AM
 */


// Field

function jobbag_tasks_field_info() {
  $info = array();

  $info['jobbag_tasks'] = array(
    'label' => t('Tasks'),
    'settings' => array(),
    'default_widget' => 'tasks_widget',
    'default_formatter' => 'tasks_formatter',
    'no ui' => TRUE
  );

  $info['jobbag_task_workflow'] = array(
    'label' => t('Task Workflow'),
    'settings' => array(
      'workflow' => NULL
    ),
    'default_widget' => 'task_workflow_widget',
    'default_formatter' => 'task_workflow_formatter'
  );

  $info['jobbag_statuses'] = array(
    'label' => t('Statuses'),
    'settings' => array(),
    'default_widget' => 'statuses_widget',
    'default_formatter' => 'statuses_formatter',
    'no ui' => TRUE
  );

  $info['jobbag_status_workflow'] = array(
    'label' => t('Status Workflow'),
    'cardinality' => 1,
    'settings' => array(
      'workflow' => NULL
    ),
    'default_widget' => 'status_workflow_widget',
    'default_formatter' => 'status_workflow_formatter'
  );

  return $info;
}

function jobbag_tasks_field_settings_form($field, $instance, $has_data) {
  $settings = $field['settings'];
  $form = array();
  if (in_array($field['type'], array('jobbag_task_workflow', 'jobbag_status_workflow'))) {
    $options = array();
    $entities = $field['type'] == 'jobbag_task_workflow' ? jobbag_task_workflow_load_multiple()
      : jobbag_status_workflow_load_multiple();

    foreach ($entities as $wid => $workflow) {
      $options[$wid] = $workflow->label();
    }

    $form['workflow'] = array(
      '#type' => 'select',
      '#title' => t('Workflow Set'),
      '#options' => $options,
      '#required' => TRUE,
      '#default_value' => $has_data ? $settings['workflow'] : NULL
    );
  }

  return $form;
}

// Widget

function jobbag_tasks_widget_info() {
  $info = array();

  $info['tasks_widget'] = array(
    'label' => t('Tasks'),
    'description' => t('Tasks associated with this type of workflow'),
    'field types' => array(
      'jobbag_tasks'
    ),
    'settings' => array(
      'cardinality' => FIELD_CARDINALITY_UNLIMITED
    ),
    'behaviors' => array(
      'multiple values' => FIELD_BEHAVIOR_CUSTOM,
      'default value' => FIELD_BEHAVIOR_DEFAULT
    )
  );

  $info['task_workflow_widget'] = array(
    'label' => t('Task Workflow'),
    'description' => t('Associates job roles and users to a set of tasks'),
    'field types' => array(
      'jobbag_task_workflow'
    ),
    'settings' => array(
      'cardinality' => 1
    ),
    'behaviors' => array(
      'multiple values' => FIELD_BEHAVIOR_CUSTOM,
      'default value' => FIELD_BEHAVIOR_NONE
    )
  );

  $info['statuses_widget'] = array(
    'label' => t('Statuses'),
    'description' => t('Statuses associated with this type of workflow'),
    'field types' => array(
      'jobbag_statuses'
    ),
    'settings' => array(
      'cardinality' => FIELD_CARDINALITY_UNLIMITED
    ),
    'behaviors' => array(
      'multiple values' => FIELD_BEHAVIOR_CUSTOM,
      'default value' => FIELD_BEHAVIOR_DEFAULT
    )
  );

  $info['status_workflow_widget'] = array(
    'label' => t('Status Workflow'),
    'description' => t('Associates a set of statuses with a task'),
    'field types' => array(
      'jobbag_status_workflow'
    ),
    'settings' => array(
      'cardinality' => 1
    ),
    'behaviors' => array(
      'multiple values' => FIELD_BEHAVIOR_CUSTOM,
      'default value' => FIELD_BEHAVIOR_NONE
    )
  );

  return $info;
}

function jobbag_tasks_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  $entity_type = $form_state['entity_type'];
  $entity = $form_state[$entity_type];
  $elements = array();

  if (in_array($field['type'], array('jobbag_task_workflow', 'jobbag_status_workflow'))) {

  }
  elseif (in_array($field['type'], array('jobbag_tasks', 'jobbag_statuses'))
    && in_array($instance['widget']['type'], array('tasks_widget', 'statuses_widget'))) {
    $entities = $instance['widget']['type'] == 'tasks_widget'
      ? jobbag_task_load_multiple() : jobbag_task_status_load_multiple();
    $elements['#theme'] = 'tasks_field_widget';
    $elements['#theme_wrappers'] = array('fieldset');
    $elements['#field_name'] = $element['#field_name'];
    $elements['#field_type'] = $field['type'];
    $elements['#widget_type'] = $instance['widget']['type'];
    $elements['#entity'] = $entity;
    $elements['#language'] = $element['#language'];
    $elements['#process'] = array('jobbag_tasks_field_widget_multiple');

    $element += array(
      '#type' => 'task_field_item',
      '#value' => $items[$delta],
      '#entity_load' => $instance['widget']['type'] == 'tasks_widget' ? 'jobbag_task_load' : 'jobbag_task_status_load'
    );

    $delta = -1;
    foreach ($items as $delta => $value) {
      $elements[$delta] = $element;
      $elements[$delta]['#default_value'] = $value['entity'];
      $elements[$delta]['#weight'] = $delta;
      unset($entities[$value['entity']]);
    }
    $delta++;
    $elements['#select_delta'] = $delta;

    $options = array();
    foreach ($entities as $id => $entity) {
      $options[$id] = $entity->label();
    }

    $element['#options'] = $options;
  }

  return $elements;
}

function jobbag_tasks_field_is_empty($item, $field) {
  if (is_array($item) && isset($item['entity'])) {
    return $item['entity'] > 0;
  }
  return $item && $item > 0;
}

function jobbag_tasks_field_widget($element, &$form_state, $form) {
  $item = $element['#value'];
  $field = field_widget_field($element, $form_state);
  $instance = field_widget_instance($element, $form_state);
  $settings = $instance['widget']['settings'];

  $parents = array_slice($element['#array_parents'], 0, -1);
  $new_path = 'job/tasks/ajax/' . implode('/', $parents) . '/' . $form['form_build_id']['#value'];
  $field_element = drupal_array_get_nested_value($form, $parents);
  $new_wrapper = $field_element['#id'] . '-ajax-wrapper';
  foreach (element_children($element) as $key) {
    if (isset($element[$key]['#ajax'])) {
      $element[$key]['#ajax']['path'] = $new_path;
      $element[$key]['#ajax']['wrapper'] = $new_wrapper;
    }
  }
  unset($element['#prefix'], $element['#suffix']);
}

function jobbag_tasks_field_widget_multiple($element, &$form_state, $form) {
  $element_children = element_children($element, TRUE);
  $count = count($element_children);

  foreach ($element_children as $delta => $key) {
    if ($key != $element['#select_delta']) {
      $element[$key]['_weight'] = array(
        '#type' => 'weight',
        '#delta' => $count,
        '#value' => $delta
      );
    }
    else {
      $element[$key]['#title'] = $element['#title'];
      $element[$key]['_weight'] = array(
        '#type' => 'hidden',
        '#default_value' => $delta
      );
    }
  }

  // Add a new wrapper around all the elements for AJAX replacement.
  $element['#prefix'] = '<div id="' . $element['#id'] . '-ajax-wrapper">';
  $element['#suffix'] = '</div>';

  return $element;
}

function _jobbag_task_widget_submit($form, $form_state) {
  $parents = array_slice($form_state['triggering_element']['#parents'], 0, -2);
  drupal_array_set_nested_value($form_state['input'], $parents, NULL);
}

// Formatters

function jobbag_tasks_formatter_info() {
  $info = array();

  $info['tasks_formatter'] = array(
    'label' => t('Tasks'),
    'description' => t('Displays the current tasks attached to a workflow'),
    'field types' => array(
      'jobbag_tasks'
    )
  );

  $info['task_workflow_formatter'] = array(
    'label' => t('Task Workflow'),
    'description' => t('Displays the current tasks within the workflow attached to a job'),
    'field types' => array(
      'jobbag_task_workflow'
    )
  );

  $info['statuses_formatter'] = array(
    'label' => t('Statuses'),
    'description' => t('Displays the statuses in a workflow'),
    'field types' => array(
      'jobbag_statuses'
    )
  );

  $info['status_workflow_formatter'] = array(
    'label' => t('Status Workflow'),
    'description' => t('Displays the current status for a task'),
    'field types' => array(
      'jobbag_status_workflow'
    )
  );

  return $info;
}

function jobbag_tasks_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();
  switch($display['type']) {
    case 'tasks_formatter':
      $ids = array();
      foreach ($items as $item) {
        $ids[] = $item['entity'];
      }
      $entities = jobbag_task_load_multiple($ids);
      $header = array(
        t('ID'),
        t('Task')
      );
      $rows = array();

      foreach ($entities as $id => $task) {
        $rows[] = array(
          $id,
          $task->label()
        );
      }

      $element[0] = array(
        '#theme' => 'table',
        '#header' => $header,
        '#rows' => $rows
      );
      break;
    case 'statuses_formatter':
      $ids = array();
      foreach ($items as $item) {
        $ids[] = $item['entity'];
      }
      $entities = jobbag_task_status_load_multiple($ids);
      $header = array(
        t('ID'),
        t('Status')
      );
      $rows = array();

      foreach ($entities as $id => $status) {
        $rows[] = array(
          $id,
          $status->label()
        );
      }

      $element[0] = array(
        '#theme' => 'table',
        '#header' => $header,
        '#rows' => $rows
      );
      break;
  }
  return $element;
}