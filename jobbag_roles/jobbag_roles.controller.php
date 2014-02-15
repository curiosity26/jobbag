<?php

class JobRoleControllerExportable extends EntityAPIControllerExportable {
    public function export($entity, $prefix = '') {
        if (is_array($entity))
            $entity = array_shift($entity);
        
        return parent::export($entity, $prefix);
    }
}

class JobRoleUIController extends EntityDefaultUIController {
  public function hook_menu() {
    $items = parent::hook_menu();
    $items[$this->path]['type']['title'] = t('Job Roles');
    $items[$this->path]['type'] = MENU_LOCAL_ACTION;
    $items[$this->path]['tab_parent'] = 'admin/jobs';
    $items[$this->path]['tab_root'] = 'admin/jobs';
    $items[$this->path]['context'] = MENU_CONTEXT_PAGE | MENU_CONTEXT_INLINE;

    return $items;
  }

  public function overviewForm($form, &$form_state) {
    drupal_set_title('Job Roles');
    return parent::overviewForm($form, $form_state);
  }
}