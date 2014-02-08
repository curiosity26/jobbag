<?php

function job_roles_job_form($form, &$form_state, $job) {
    $roles = job_roles_load_multiple();
    $form_state['storage']['entity'] = $job;
    drupal_set_title($job->getJobNumber()."'s Roles");
    
    $form['role_users']['#tree'] = TRUE;
    
    $form['roles'] = array(
        '#type' => 'value',
        '#value' => $roles
    );
    
    foreach ($roles as $role) {
        $users = array();
        
        if (!empty($job->roles[$role->rid]) && !empty($job->roles[$role->rid]->users))
            $users = $job->roles[$role->rid]->users;
        
        $form['role_name'][$role->rid] = array(
            '#type' => 'markup',
            '#markup' => $role->label
        );
        
        $users = job_roles_eligible_users($role);
        
        if ($role->rid == 1)
            unset($users[0]);
        
        $form['role_users'][$role->rid] = array(
            '#type' => 'select',
            '#multiple' => $role->cardinality != 1,
            '#options' => $users,
            '#default_value' => !empty($job->roles[$role->rid]->users) ? $job->roles[$role->rid]->users : 0,
            '#size' => $role->cardinality == 1 ? 1 : min(10, count($users)),
            '#tree' => TRUE
        );
    }
    
    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Save roles')
    );
    
    return $form;
}

function job_roles_job_form_submit(&$form, &$form_state) {
    $job = $form_state['storage']['entity'];
    $roles = $form_state['values']['roles'];
    $users = array();
    
    if (is_object($job)) {
        foreach ($form_state['values']['role_users'] as $rid => $uids) {
            $jrid = db_select('jobbag_job_roles', 'r')->distinct()->fields('r', array('jrid'))->condition(db_and()->condition('jid', $job->jid)->condition('rid', $rid))->execute()->fetchField();
            $record = new stdClass();
            $key = array();
            
            if ($jrid) {
                $key[] = 'jrid';
                $record->jrid = $jrid;
            }
            
            $record->rid = $rid;
            $record->jid = $job->jid;
            $record->users = !is_array($uids) ? array($uids) : $uids;
            
            $success = drupal_write_record('jobbag_job_roles', $record, $key);
            
            unset($jrid);
            unset($record);
            unset($key);
            
            if (!$success) {
                form_error($form['role_users'][$rid], 'Unable to save job role settings');
                $form_state['rebuild'] = TRUE;
                break;
            }
        }
    }
    
    drupal_set_message('Successfully saved job role settings');
    
    $context['old_roles'] = $job->roles;
    job_load_roles($job, TRUE);
    
    $actions = trigger_get_assigned_actions('jobbag_roles_add_users');
    $context['new_roles'] = $job->roles;
    if ($actions)
        actions_do($actions, $job, $context, $context['old_roles'], $context['new_roles']);
}