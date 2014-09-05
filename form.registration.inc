<?php

/**
 * Form callback for event registration form
 */
function munroe_event_form($form_state) {

    $form   = array();
    $events = _munroe_event_form_get_events();

    # unable to proceed if we can't get contact for the currently logged in user,
    # lookup function will post an error if so
    if (!$you = _munroe_event_form_get_you())
        return;

    # select event(s) from querystring data 
    if (isset($_GET['id']) and is_numeric($_GET['id']))
        $defaults = array(
            'contact_a_events' => array($_GET['id'] => $_GET['id']),
            'contact_b_events' => array($_GET['id'] => $_GET['id'])
        );

    # or repopulate form from session data if 'Back' button pressed
    elseif (isset($_SESSION['registration_info']))
        $defaults = &$_SESSION['registration_info'];

    $spouse = _munroe_event_form_get_your_spouse();
 
    $form['contact_a'] = array(
        '#type'  => 'fieldset',
        '#title' => t('You')
    );

    $form['contact_a']['contact_a_first_name'] = array(
        '#type'   => 'item',
        '#title'  => t('First name') . ':',
        '#markup' => $you['first_name']
    );

    $form['contact_a']['contact_a_last_name'] = array(
        '#type'   => 'item',
        '#title'  => t('Last name') . ':',
        '#markup' => $you['last_name']
    );

    $form['contact_a']['contact_a_email'] = array(
        '#type'   => 'item',
        '#title'  => t('Email') . ':',
        '#markup' => $you['email']
    );

    $form['contact_a']['contact_a_events'] = array(
        '#type'          => 'checkboxes',
        '#title'         => t('Register for event(s)'),
        '#options'       => $events,
        '#default_value' => isset($defaults['contact_a_events']) ? $defaults['contact_a_events'] : array()
    );

    if ($spouse) {

        $form['contact_b'] = array(
            '#type'  => 'fieldset',
            '#title' => t('Your Spouse')
        );

        $form['contact_b']['contact_b_first_name'] = array(
            '#type'   => 'item',
            '#title'  => t('First name') . ':',
            '#markup' => $spouse['first_name']
        );

        $form['contact_b']['contact_b_last_name'] = array(
            '#type'   => 'item',
            '#title'  => t('Last name') . ':',
            '#markup' => $spouse['last_name']
        );

        $form['contact_b']['contact_b_email'] = array(
            '#type'   => 'item',
            '#title'  => t('Email') . ':',
            '#markup' => $spouse['email']
        );

        $form['contact_b']['contact_b_events'] = array(
            '#type'          => 'checkboxes',
            '#title'         => t('Register for event(s)'),
            '#options'       => $events,
            '#default_value' => isset($defaults['contact_b_events']) ? $defaults['contact_b_events'] : array()
        );

    }

    $form['submit'] = array(
        '#type'       => 'submit',
        '#value'      => t('Register »'),
        '#validate'   => array('munroe_event_form_validate'),
        '#attributes' => array('class' => array('btn-primary'))
    );

    $form['#submit'][] = 'munroe_event_form_submit';

    return $form;

}

/**
 * Event registration form validate handler
 */
function munroe_event_form_validate($form, $form_state) {

    # if only contact A displayed and no contact A events selected, post error
    if (!isset($form_state['values']['contact_b_events']) and !array_sum($form_state['values']['contact_a_events']))
        form_set_error('contact_a_events', t('Please select at least one event'));
    
    # if both contacts displayed and no events selected for either contact, post error
    elseif (
        isset($form_state['values']['contact_b_events']) 
        and !array_sum($form_state['values']['contact_a_events'])
        and !array_sum($form_state['values']['contact_b_events'])
    ) {
        form_set_error('contact_a_events');
        form_set_error('contact_b_events');
        drupal_set_message(t('Please select at least one event'), 'error');
    }

}

/**
 * Event registration form submission handler
 */
function munroe_event_form_submit($form, $form_state) {

    # store registration info and redirect to confirmation page
    $_SESSION['registration_info'] = $form_state['values'];
    drupal_goto('event-registration/confirm');

}