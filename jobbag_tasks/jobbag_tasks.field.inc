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

  return $info;
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

  return $info;
}

function jobbag_tasks_widget_settings_form($field, $instance) {
  $widget = $instance['widget'];
  $settings = $widget['settings'];
  $items = (array) $settings['task_types'];
  $types = jobbag_task_type_load_multiple();
  $form = array();
  $form['task_types'] = array(
    '#theme' => 'tasks_field_widget_settings',
    '#theme_wrappers' => array('fieldset'),
    '#field_name' => $field['field_name'],
    '#field_type' => $field['type'],
    '#process' => array('jobbag_tasks_widget_process_multiple')
  );

  $element_info = element_info('task_field_item');
  $element = array(
    '#type' => 'task_field_item',
    '#process' => array_merge($element_info['#process'], array('jobbag_tasks_widget_process'))
  );

  $delta = -1;
  foreach ($items as $delta => $value) {
    $form['task_types'][$delta] = $element;
    $form['task_types'][$delta]['#default_value'] = $value['entity'];
    $form['task_types'][$delta]['#weight'] = $delta;
    unset($types[$value['entity']]);
  }
  $delta++;
  $form['task_types']['#select_delta'] = $delta;

  $options = array();
  foreach ($types as $id => $entity) {
    $options[$id] = $entity->label();
  }

  $form['task_types']['#options'] = $options;

  return $form;
}

function jobbag_tasks_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  $entity_type = $form_state['entity_type'];
  $entity = $form_state[$entity_type];
  $settings = $instance['widget']['settings'];
  $types = $settings['task_types'];
  $elements = array(
    '#theme' => 'task_field_widget',
  );

  $element += array(
    '#type' => 'task_field_widget_item',
    '#job' => $entity
  );

  foreach ($items as $delta => $item) {
    $elements[$delta] = $element + array(
      '#task' => isset($item['id']) ? $item['id'] : 0,
      '#task_type' => isset($item['type']) ? $item['type'] : $types[$delta],
      '#weight' => $delta,
      '#roles' => isset($item['roles']) ? $item['roles'] : array(),
      '#accounts' => isset($item['accounts']) ? $item['accounts'] : array(),
      '#status' => isset($item['status']) ? $item['status'] : 'new',
    );
  }

  return $elements;
}

function jobbag_tasks_field_is_empty($item, $field) {
  if (is_array($item)) {
    return !isset($item['type']);
  }
  return !$item;
}

function jobbag_tasks_widget_process($element, &$form_state, $form) {
  $parents = array_slice($element['#array_parents'], 0, -1);
  $new_path = 'job/tasks/ajax/' . implode('/', $parents) . '/' . $form['form_build_id']['#value'];
  $field_element = drupal_array_get_nested_value($form, $parents);
  $new_wrapper = $field_element['#id'] . '-ajax-wrapper';
  foreach (array('add', 'remove') as $key) {
    if (isset($element[$key]['#ajax'])) {
      $element[$key]['#ajax']['path'] = $new_path;
      $element[$key]['#ajax']['wrapper'] = $new_wrapper;
    }
  }
  unset($element['#prefix'], $element['#suffix']);
  return $element;
}

function jobbag_tasks_widget_process_multiple($element, &$form_state, $form) {
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

  foreach (array('add', 'remove') as $key) {
    $element[$key]['#submit'][] = '_jobbag_task_widget_submit';
    $element[$key]['#limit_validation_errors'] = array(array_slice($element['#parents'], 0, -1));
  }

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
    'description' => t('Displays the current tasks attached to a job'),
    'field types' => array(
      'jobbag_tasks'
    )
  );

  return $info;
}

function jobbag_tasks_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $elements = array();

  if ($display['type'] == 'tasks_formatter') {
    foreach ($items as $delta => $item) {
      $elements['#theme'] = 'tasks_formatter_items';
      $elements[$delta] = entity_build_content('jobbag_task', $item['task'], 'field', $langcode);
    }
  }

  return $elements;
}

/* Load */

function jobbag_tasks_field_load($entity_type, $entities, $field, $instances, $langcode, &$items, $age) {
  if ($field['type'] == 'jobbag_tasks') {
    foreach ($items as &$item) {
      if ($item['id'] != 0) {
        $item['task'] = jobbag_task_load($item['id']);
        $item['roles'] = $item['task']->roles;
        $item['accounts'] = $item['task']->accounts;
        $item['status'] = $item['task']->status;
      }
    }
  }
}

/* Insert */

function jobbag_tasks_field_insert($entity_type, $entity, $field, $instance, $langcode, &$items) {
  if ($field['type'] == 'jobbag_tasks') {
    foreach ($items as &$item) {
      $task_type = jobbag_task_type_load($item['type']);
      $settings = array(
        'type' => $task_type->type,
        'roles' => $item['roles'],
        'accounts' => $item['accounts'],
        'job' => $entity->identifier(),
        'status' => $item['status']
      );

      $item['task'] = entity_create('jobbag_task', $settings);
      if (!!$item['task']) {
        entity_save('jobbag_task', $item['task']);
        $item['id'] = $item['task']->identifier();
      }
    }
  }
}

/* Update */

function jobbag_tasks_field_update($entity_type, $entity, $field, $instance, $langcode, &$items) {
  if ($field['type'] == 'jobbag_tasks') {
    foreach($items as &$item) {
      if ($item['id'] == 0) {
        $task_type = jobbag_task_type_load($item['type']);
        $settings = array(
          'type' => $task_type->type,
          'roles' => $item['roles'],
          'accounts' => $item['accounts'],
          'job' => $entity->identifier(),
          'status' => $item['status']
        );

        $item['task'] = entity_create('jobbag_task', $settings);
        if (!!$item['task']) {
          entity_save('jobbag_task', $item['task']);
          $item['id'] = $item['task']->identifier();
        }
      }
      else {
        $task = &$item['task'];
        $task->roles = $item['roles'];
        $task->accounts = $item['accounts'];
        $task->status = $item['status'];
        entity_save('jobbag_task', $task);
      }
    }
  }
}

/* Delete */

function jobbag_tasks_field_delete($entity_type, $entity, $field, $instance, $langcode, &$items) {
  if ($field['type'] == 'jobbag_tasks') {
    $ids = db_select('jobbag_tasks', 't')
      ->fields('t', array('id'))
      ->condition('job', $entity->identifier())
      ->execute()->fetchAllAssoc('id');

    if (!empty($ids)) {
      entity_delete_multiple('jobbag_task', array_keys($ids));
    }
  }
}