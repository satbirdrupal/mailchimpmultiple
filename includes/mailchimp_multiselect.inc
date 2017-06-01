<?php


/**
 * This is helper function used for get all child list of a specific field
 * @param null $field_name
 *  This is filed name for which we need all lists this is optional if this is not pass then this function will return all lists from table
 * @return array|bool
 */
function mailchimp_multiselect_get_all_lists($field_name = NULL)
{
    $lists = false;
    $query = db_select('mailchimp_multiselect_list', 'mml');
    $query->fields('mml', array('mc_list_id'));

    if ($field_name) {
        $query->condition('mml.field_name', $field_name, '=');
    }

    $result = $query->execute();
    while ($record = $result->fetchAssoc()) {
        $lists[] = $record['mc_list_id'];
    }

    return $lists;
}

/**
 * This is helper function that will return all child list of a mailchimp list
 *  In multiple widget we are assuming first list as master list actually in field setting that is only list that save for field rest lists we store in
 *  'mailchimp_multiselect_list' table as child lists this function will accept a parameter of master list id and will return all child lists.
 * @param $list
 * @return array|bool
 */
function mailchimp_multiselect_get_child_lists($list)
{
    $lists = false;
    if (!empty($list)) {
        $query = db_select('mailchimp_multiselect_list', 'mml');
        $query->fields('mml', array('mc_list_id'));
        $query->condition('mml.mc_list_master_id', $list, '=');
        $result = $query->execute();
        while ($record = $result->fetchAssoc()) {
            $lists[] = $record['mc_list_id'];
        }
    }

    return $lists;
}

/**
 * This is helper function that will accept a list id and return field name if that list is attached in any field.
 * @param $list
 *  This is mailchimp list id
 * @return bool
 */
function mailchimp_multiselect_get_list_related_field($list)
{
    $field_name = false;
    if (!empty($list)) {
        $query = db_select('mailchimp_multiselect_list', 'mml');
        $query->fields('mml', array('field_name'));
        $query->condition('mml.mc_list_id', $list, '=');
        $result = $query->execute();
        while ($record = $result->fetchAssoc()) {
            $field_name = $record['field_name'];
        }
    }

    return $field_name;
}

