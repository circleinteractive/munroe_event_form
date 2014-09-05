<?php

/**
 * Form callback for event confirmation form
 */
function munroe_event_form_confirmation($form_state) {

    $form = array();
    
    if (!$registration = &$_SESSION['registration_info'])
        drupal_goto('event-registration');

    # if the primary contact has selected events, display them
    if (isset($registration['contact_a_events'])
        and array_sum($registration['contact_a_events'])  
        and $you = _munroe_event_form_get_you()
    ) {

        $form['contact_a_name'] = array(
            '#type'   => 'item',
            '#title'  => t('Contact name') . ':',
            '#markup' => $you['first_name'] . ' ' . $you['last_name']
        );

        $form['contact_a_events'] = array(
            '#type'   => 'item',
            '#markup' => theme('munroe_event_table', array(
                'events' => _munroe_event_form_load_events(
                    $registration['contact_a_events']
                )
            ))
        );

    }

    # if spouse has selected events, display them
    if (isset($registration['contact_b_events']) 
        and array_sum($registration['contact_b_events']) 
        and $spouse = _munroe_event_form_get_your_spouse()
    ) {
        
        $form['contact_b_name'] = array(
            '#type'   => 'item',
            '#title'  => t('Contact name') . ':',
            '#markup' => $spouse['first_name'] . ' ' . $spouse['last_name']
        );

        $form['contact_b_events'] = array(
            '#type'   => 'item',
            '#markup' => theme('munroe_event_table', array(
                'events' => _munroe_event_form_load_events(
                    $registration['contact_b_events']
                )
            ))
        );
    
    }

    $form['back'] = array(
        '#type'       => 'submit',
        '#value'      => t('« Back'),
        '#submit'     => array('munroe_event_form_confirmation_back'),
        '#attributes' => array('style' => 'float:left')
    );

    $form['confirm'] = array(
        '#type'       => 'submit',
        '#value'      => t('Confirm »'),
        '#submit'     => array('munroe_event_form_confirmation_confirm'),
        '#attributes' => array('style' => 'float:right')
    );

    $form['clear'] = array('#markup' => '<div style="clear:both"></div>');

    return $form;

}

function munroe_event_form_confirmation_back() {
    drupal_goto('event-registration');
}

function munroe_event_form_confirmation_confirm($form, $form_state) {

    # register participants
    if (!$you = _munroe_event_form_get_you())
        return;

    if (!$registration = &$_SESSION['registration_info'])
        drupal_goto('event-registration');

    # record contact id, event id, participant id for sending emails / tickets
    $event_registrations = array();
    $events              = array();

    foreach ($registration['contact_a_events'] as $key => $event_id) {
        
        # ignore and remove if event unselected
        if (!$event_id) {
            unset($registration['contact_a_events'][$key]);
            continue;
        }
        
        try {
            $result = civicrm_api3('participant', 'create', array(
                'event_id'      => $event_id,
                'contact_id'    => $you['id'],
                'status_id'     => 1,
                'role_id'       => 1,
                'register_date' => date('Y-m-d h:i:s'),
                'source'        => ts('Event Registration Webform'),
                'sequential'    => 1
            ));
        } catch (CiviCRM_API3_Exception $e) {
            watchdog(__DIR__, t('Unable to create participant record for contact id !cid: @excuse', array(
                '!cid'    => $you['id'],
                '@excuse' => $e->getMessage() 
            )));
            CRM_Core_Error::fatal("
                Unable to a create participant record for one of your chosen events. 
                Please contact the site administrator for assistance."
            );
        }

        $event_registrations[] = array(
            'contact_id'     => $you['id'],
            'event_id'       => $event_id,
            'participant_id' => $result['values'][0]['id']
        );

        $events[$event_id] = _munroe_event_form_get_event($event_id);

    }

    if ($spouse = _munroe_event_form_get_your_spouse()) {

        foreach ($registration['contact_b_events'] as $key => $event_id) {
            
            # ignore and remove if event unselected
            if (!$event_id) {
                unset($registration['contact_b_events'][$key]);
                continue;
            }
            
            try {
                $result = civicrm_api3('participant', 'create', array(
                    'event_id'      => $event_id,
                    'contact_id'    => $spouse['id'],
                    'status_id'     => 1,
                    'role_id'       => 1,
                    'register_date' => date('Y-m-d h:i:s'),
                    'source'        => ts('Event Registration Webform'),
                    'sequential'    => 1
                ));
            } catch (CiviCRM_API3_Exception $e) {
                watchdog(__DIR__, t('Unable to create participant record for contact id !cid: @excuse', array(
                    '!cid'    => $spouse['id'],
                    '@excuse' => $e->getMessage() 
                )));
                CRM_Core_Error::fatal("
                    Unable to a create participant record for one of your chosen events. 
                    Please contact the site administrator for assistance."
                );
            }

            $event_registrations[] = array(
                'contact_id'     => $spouse['id'],
                'event_id'       => $event_id,
                'participant_id' => $result['values'][0]['id']
            );

            $events[$event_id] = _munroe_event_form_get_event($event_id);

        }

    }
 
    # send email
    $params = array(
        'groupName' => 'msg_tpl_workflow_event',
        'valueName' => 'event_multi_receipt',
        'contactId' => $you['id'],
        'tplParams' => array(
            'registration_info' => $event_registrations,
            'events'            => $events,
            'contact_a'         => $you,
            'contact_b'         => isset($spouse) ? $spouse : array(),
            'contact_a_events'  => $registration['contact_a_events'],
            'contact_b_events'  => isset($registration['contact_b_events']) ? $registration['contact_b_events'] : array()
        ),
        'from'    => 'test@munroe.prestige55.org',
        'toName'  => $you['display_name'],
        'toEmail' => $you['email']
    );
    
    watchdog('andyw', 'Mailer params = <pre>' . print_r($params, true) . '</pre>');
    CRM_Core_BAO_MessageTemplate::sendTemplate($params);

    # unset registration info so registration form will be cleared on next visit
    unset($_SESSION['registration_info']);

    # redirect to thankyou page
    drupal_goto('event-registration/complete');

}