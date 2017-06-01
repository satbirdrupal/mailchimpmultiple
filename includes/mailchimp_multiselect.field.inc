<?php


/**
 * Implements hook_field_formatter_info().
 */
function mailchimp_multiselect_field_formatter_info() {
    return array(
        'mailchimp_lists_field_subscribe' => array(
            'label' => t('Subscription Form'),
            'field types' => array('mailchimp_lists_subscription'),
            'settings' => array(
                'show_interest_groups' => FALSE,
            ),
        ),
        'mailchimp_lists_subscribe_default' => array(
            'label' => t('Subscription Info'),
            'field types' => array('mailchimp_lists_subscription'),
            'settings' => array(
                'show_interest_groups' => FALSE,
                'interest_groups' => array(),
            ),
        ),
    );
}

/**
 * Implements hook_field_formatter_settings_form().
 */
function mailchimp_multiselect_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {
    $display = $instance['display'][$view_mode];
    $settings = $display['settings'];

    $element = array();
    $element['show_interest_groups'] = array(
        '#title' => t('Show Interest Groups'),
        '#type' => 'checkbox',
        '#description' => $instance['settings']['show_interest_groups'] ? t('Check to display interest group membership details.') : t('To display Interest Groups, first enable them in the field instance settings.'),
        '#default_value' => $instance['settings']['show_interest_groups'] && $settings['show_interest_groups'],
        '#disabled' => !$instance['settings']['show_interest_groups'],
    );

    return $element;
}

/**
 * Implements hook_field_formatter_settings_summary().
 */
function mailchimp_multiselect_field_formatter_settings_summary($field, $instance, $view_mode) {
    $display = $instance['display'][$view_mode];
    $settings = $display['settings'];
    if ($settings['show_interest_groups'] && $instance['settings']['show_interest_groups']) {
        $summary = t('Display Interest Groups');
    }
    else {
        $summary = t('Hide Interest Groups');
    }

    return $summary;
}

/**
 * Implements hook_field_formatter_view().
 */
function mailchimp_multiselect_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
    $element = array();
    $use_interest_groups = ($instance['settings']['show_interest_groups'] && $display['settings']['show_interest_groups']);
    $mc_lists[] = mailchimp_get_list($field['settings']['mc_list_id'], $use_interest_groups);
    $email = mailchimp_lists_load_email($instance, $entity, FALSE);
    $child = mailchimp_multiselect_get_child_lists($field['settings']['mc_list_id']);

    /* if multiple lists are selected in field */
    if(!empty($child)) {
        foreach ($child as $child_list) {
            $mc_lists[] = mailchimp_get_list($child_list, $use_interest_groups);
        }
    }

    $i = 0;
    foreach($mc_lists as $mc_list) {
        // Display subscription form if accessible.
        if ($display['type'] == 'mailchimp_lists_field_subscribe' && $email && entity_access('edit', $entity_type, $entity)) {
            $field_form_id = 'mailchimp_lists_' . $field['field_name'] . '_form';
            $element[$i] = drupal_get_form($field_form_id, $instance, $display['settings'], $entity, $field);
        }
        else {
            if ($email) {
                $member_info = mailchimp_get_memberinfo($mc_list->id, $email);
                $member_status = (isset($member_info->status)) ? $member_info->status : NULL;
                switch ($member_status) {
                    case 'subscribed':
                        $status = t('Subscribed to %list', array('%list' => $mc_list->name));
                        break;
                    case 'pending':
                        $status = t('Pending confirmation for %list', array('%list' => $mc_list->name));
                        break;
                    default:
                        $status = t('Not subscribed to %list', array('%list' => $mc_list->name));
                }
            }
            else {
                $status = t('Invalid email configuration.');
            }
            $status_item = array();
            $status_item[] = $status;
            $element[$i]['status'] = array(
                '#theme' => 'item_list',
                '#items' => $status_item,
                '#type' => 'ul',

                '#description' => t('@mc_list_description', array('@mc_list_description' => $instance['description'])),
            );
            if ($use_interest_groups && !empty($mc_list->intgroups)) {

                $element[$i]['interest_groups'] = array(
                    '#type' => 'fieldset',
                    '#title' => t('Interest Groups'),
                    '#weight' => 100,
                );

                foreach ($mc_list->intgroups as $interest_group) {
                    $items = array();
                    foreach ($interest_group->interests as $interest) {
                        if (isset($member_info->interests->{$interest->id}) && ($member_info->interests->{$interest->id} === TRUE)) {
                            $items[] = $interest->name;
                        }
                    }

                    if (count($items) > 0) {
                        $element[$i]['interest_groups'][$interest_group->id] = array(
                            '#title' => $interest_group->title,
                            '#theme' => 'item_list',
                            '#items' => $items,
                            '#type' => 'ul',
                        );
                    }
                }
            }
        }

        $i++;
    }

    return array($element);
}