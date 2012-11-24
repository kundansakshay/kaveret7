<?php
include ('mcapi_forms.inc');

/**
 * implements hook_help
 */
function mcapi_forms_help($path, $args) {
  if (substr($path, 0, 22) == 'admin/accounting/forms') {
    if (strlen($path) > 22) {
      $help[] = t('Edit transaction form.');
      if (module_exists('locale')) {
        $help[] = t('To work in many languages this form should be exported to a custom module, then not overwritten, and the strings wrapped in t().');
      }
      return implode(' ', $help);
    }
    else {
      return t('Create transaction forms for specific use-cases. Use with caution as not all permutations are meaningful.');
    }
  }
}

/*
 * This is the form-builder form
 * For every field in the transaction form, this form allows it to be configured.
 * Its not geometrically perfect, but covers a lot of use cases without getting too complicated
 */
function _edit_mcapi_form(&$form, &$form_state) {
  $mcapi_form = &$form_state['item']->data;
  $form['#prefix'] = '<div id = "form-wrapper">';
  $form['#suffix'] = '</div>';
  //I think ctools is supposed to be handling this
  $form['info']['name']['#type'] = 'machine_name';
  $form['info']['name']['#machine_name']['exists'] = 'mcapi_forms_exists';
  $form['info']['name']['#maxlength'] = 32;
  $perspectives = array(
    3 => t('3rd Party'),
    1 => t('1st Party')
  );
  //to cover up the fact that we can only reload the form once
  if (isset($form_state['values']['perspective'])) {
    //only include the selected option
    $form['perspective']['#options'] = array($form_state['values']['perspective'] => $perspectives[$form_state['values']['perspective']]);
    $form['perspective']['#description'] = 'Technical limitations prohibit this being flipped more than once. Reload the whole page to change it back';
  }

  $form['architecture'] = array(
    '#title' => t('Architecture'),
    '#description' => t("Every form is a menu callback with its own access control. Don't forget currencies have their own access control as well."),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => !empty($mcapi_form),
    '#weight' => 3,
    'path' => array(
      '#title' => t('Path'),
      '#description' => t("Unique path to this form. Don't forget you can use a path alias over this"),
      '#type' => 'textfield',
      '#default_value' => isset($mcapi_form['architecture']['path']) ? $mcapi_form['architecture']['path'] : '',
      '#required' => TRUE,
      '#weight' => 1
    ),
    'menu_link' =>array(
      '#title' => t('Menu link location'),
      '#description' => t('Choose the menu, or menu item-type.') .' '. t('Some choices may not produce a menu item') .' '.
        t("For more info, see 'type' info at !link", array('!link' => l('drupal.org', 'http://api.drupal.org/api/drupal/modules--system--system.api.php/function/hook_menu/7'))),
      '#type' => 'select',
      '#default_value' => isset($mcapi_form['architecture']['menu_link']) ? $mcapi_form['architecture']['menu_link'] : 'MENU_CALLBACK',
      '#options' => drupal_map_assoc(array_keys(menu_get_menus())) + array(
        MENU_CALLBACK => t('No menu item'),
        MENU_LOCAL_TASK => 'MENU_LOCAL_TASK',
        MENU_DEFAULT_LOCAL_TASK => 'MENU_DEFAULT_LOCAL_TASK',
        MENU_LOCAL_ACTION => 'MENU_LOCAL_ACTION'
      ),
      '#element_validate' => array(),
      '#weight' => 2
    ),
    'access' =>array(
      '#title' => t('Access control'),
      '#description' => t("In addition to currency access control, and block access control, access to this form can be restricted."),
      '#type' => 'select',
      '#default_value' => isset($mcapi_form['architecture']['access']) ? $mcapi_form['architecture']['access'] : 'currency',
      '#options' => _mcapi_user_chooser_access_options(),
      '#element_validate' => array(),
      '#weight' => 3
    ),
    'use_block' =>  array(
      '#title' => t('Make block available'),
      '#description' => t('The block is named according the path.') .' '.
        t('Remember that blocks have their own caching and access control.'),
      '#type' => 'checkbox',
      '#default_value' => isset($mcapi_form['architecture']['use_block']) ? $mcapi_form['architecture']['use_block'] : '',
      '#element_validate' => array(),
      '#weight' => 4
    )
  );

  $form['perspective'] = array(
    '#title' => t('perspective'),
    '#description' => t('Whether this is completed by a 3rd party or a participant.'),
    '#type' => 'radios',
    '#options' => $perspectives,
    '#default_value' => isset($mcapi_form['perspective']) ? $mcapi_form['perspective'] : 3,
    '#weight' => 5,
    //this ajax only works once then the form #id increments to ctools-export-ui-edit-item-form--2
    '#ajax' => array(
      'callback' => 'mcapi_forms_ajax_settings',
      'wrapper' => 'form-wrapper'
    ),
  );

  //following section of the form allows the admin to handle the individual fields of the transaction form.
  //the fields are handled here one in each tab, each field having some shared settings and some specific ones.
  $form['mcapi_form_fields'] = array(
    '#type' => 'vertical_tabs',
    '#weight' => 6,
    '#prefix' => '<style>.vertical-tabs-panes div.form-type-radio{display:inline;}.vertical-tabs .form-type-textfield input{width:inherit;}</style>',
  );
  //the participants of the transaction are either payer/payee or 1stperson/secondperson. This is called the 'perspective' of the form.
  $perspective = isset($form_state['values']['perspective']) ? $form_state['values']['perspective'] : $form['perspective']['#default_value'] ;

  //load the transaction form so we can re-use the form field declarations
  $array = array();
  $transaction_fields = transaction_form($array, $array);
  foreach (element_children($transaction_fields) as $fieldname) {
    $form[$fieldname] = (array)@$form[$fieldname];//this is good for new forms with values
    $mcapi_form[$fieldname] = (array)@$mcapi_form[$fieldname];//in case the form elements are controlled elsewhere and not preset here
    switch ($fieldname) {
      case 'type':
      case 'state':
      case 'creator':
      case 'buttons':
        break;
      case 'payer':
        if ($perspective == 1) {
          $transaction_fields[$fieldname]['#title'] = t('Other user');
          //convert the payer field to a seconderson field
          $form['secondperson'] = _mcapi_form_config_fieldset($mcapi_form['secondperson'], $transaction_fields['payer'], 'secondperson');
          $form['secondperson']['user_chooser_config'] = _mcapi_user_chooser_config_field($mcapi_form['secondperson']);
          $preset = array(
            '#title' => t('Direction'),
            '#description' => t("Either 'incoming' or 'outgoing' relative to the logged in user"),
            '#type' => $mcapi_form['direction']['widget'],
            '#options' => array(
                '' => t('Neither'),
                'incoming' => $mcapi_form['direction']['incoming'],
                'outgoing' => $mcapi_form['direction']['outgoing'],
            )
          );
          $form['direction'] = _mcapi_form_config_fieldset($mcapi_form['direction'], $preset, 'direction')
            +_mcapi_direction_config_fields($mcapi_form['direction']);
          break;
        }
        //then run on
      case 'payee':
        if ($perspective == 3) {
          $form[$fieldname] = _mcapi_form_config_fieldset($mcapi_form[$fieldname], $transaction_fields[$fieldname], $fieldname);
          $form[$fieldname]['user_chooser_config'] = _mcapi_user_chooser_config_field(@$mcapi_form[$fieldname]);
        }
        break;

      default: //this covers all fieldAPI fields
        $instance = field_read_instance('transaction', $fieldname, 'transaction');
        $default_items = &$mcapi_form[$fieldname];
        $empty_field = $transaction_fields[$fieldname][LANGUAGE_NONE];
        $form[$fieldname] = _mcapi_form_config_fieldset($default_items, $empty_field, $fieldname);
    }
  }
  $form['worth']['preset']['#required'] = FALSE;
  $form['worth']['#description'] = '<br />'. t('Leave blank to exclude the currency from the form. Otherwise preset to zero or some other value');
  $form['worth']['preset']['#element_validate'] = array('mcapi_form_validate_worth_preset');

  //we need to know the view modes of the transaction entity, to choose for the preview page
  $entity = entity_get_info('transaction');
  foreach ($entity['view modes'] as $name => $mode) {
    if (in_array($name, array('search_index', 'search_result')))continue;
    $view_modes[$name] = $mode['label'];
  }

  $form['experience'] = array(
    '#title' => t('User experience'),
    '#description' => t('Deteriming how the form and the confirmation work for the user'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#type' => 'fieldset',
    '#weight' => 15,
    'title' => array(
      '#title' => t('Title'),
      '#description' => t('For use in block title and menu links.') .' '. t('Plain text') . t('Leave blank to jump to the confirmation stage.'),
      '#type' => 'textfield',
      '#default_value' => isset($mcapi_form['experience']['title']) ? $mcapi_form['experience']['title'] : '',
      '#weight' => 1,
    ),
    'template' => array(
      '#title' => t('Main form'),
      '#description' => implode('<br />', array(
          t('Write HTML here to layout the form, using the tokens below.')
        )),
      '#type' => 'textarea',
      '#rows' => 6,
      '#default_value' => isset($mcapi_form['experience']['template']) ? $mcapi_form['experience']['template'] : '',
      '#weight' => 2,
    ),
    'token_tree' => array(
      '#theme' => 'token_tree',
      '#token_types' => array('mcapiform'),
      '#global_types' => FALSE,
      '#weight' => 3,
    ),
    'sure_title' => array(
      '#title' => t('Confirmation page title'),
      '#description' => t('Plain text'),
      '#type' => 'textfield',
      '#default_value' => isset($mcapi_form['experience']['sure_title']) ? $mcapi_form['experience']['sure_title'] : '',
      '#weight' => 4,
    ),
    'sure_format' => array(
      '#title' => t('Confirmation preview mode'),
      '#type' => 'radios',
      '#options' => $view_modes,
      '#default_value' => isset($mcapi_form['experience']['sure_format']) ? $mcapi_form['experience']['sure_format'] : 'certificate',
      '#weight' => 5,
    ),
    'sure_template' => array(
      '#title' => '('. t('Confirmation page') .')',
      '#description' => t('If using tokens mode, write HTML here to layout the confirmation, using the tokens above.'),
      '#type' => 'textarea',
      '#rows' => 4,
      '#default_value' => @$mcapi_form['experience']['sure_template'],
      '#weight' => 6,
    ),
  );
  $form['submission'] = array(
    '#title' => t('Form submission'),
    '#description' => t("Settings for @field", array('@field' => t('Submit'))),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => empty($mcapi_form),
    'button' => array(
      '#title' => t('Save button'),
      '#description' => t("The text to appear on the 'save' button, or the absolute url of an image"),
      '#type' => 'textfield',
      '#default_value' => isset($mcapi_form['submission']['button']) ? $mcapi_form['submission']['button'] : t('Submit'),
      '#required' => TRUE,
    ),
    'redirect' => array(
      '#title' => t('Destination'),
      '#description' => t('Goes to the new transaction certificate by default'),
      '#type' => 'textfield',
      '#default_value' => isset($mcapi_form['submission']['redirect']) ? $mcapi_form['submission']['redirect'] : '',
      '#element_validate' => array('mcapi_path_validate'),
      '#weight' => 1,
    ),
    'feedback' => array(
      '#title' => t('Message'),
      '#description' => t('Contents of the message box'),
      '#type' => 'textfield',
      '#default_value' => isset($mcapi_form['submission']['feedback']) ? $mcapi_form['submission']['feedback'] : '',
      '#weight' => 1,
    ),
    '#weight' => 20,
  );
}

/*
 * make a fieldset for each of the fields on the transaction form
 * Each fieldset will include a field to set a default value
 * $mcapi_form_field - the saved section of the mcapi form
 * $empty_element - an empty widget to put the #default_value in
 * $key - the fieldname
 */
function _mcapi_form_config_fieldset($field_defaults, array $empty_element, $fieldname) {
  static $w;
  $preset = &$field_defaults['preset'];
  mcapi_unset_recursive($empty_element, '#required');
  mcapi_unset_recursive($empty_element, '#element_validate');
  $element = array(
    '#title' => $empty_element['#title'],
    '#type' => 'fieldset',
    '#group' => 'mcapi_form_fields',
    '#weight' => $w++,
    'preset' => array(
      '#title' => t('Default value'),
    ) + $empty_element,
  );
  //set the defaults from the saved object.
  //Need to do it differently according to whether this is a fieldAPI field or one of the basic transaction properties
  if (isset($empty_element['#entity'])) {//this is for fieldAPI checkboxes and radio buttons
    if (isset($preset)) {
      if ($fieldname == 'worth') {
        foreach ($preset as $delta => $item) {
          $element['preset'][$delta]['#default_value'] = $item;
        }
      }
      else $element['preset']['#default_value'] = $preset;
    }
  }
  elseif (element_children($empty_element)) {//this is a normal fieldAPI field - not checkboxes or radios
    foreach (element_children($empty_element) as $delta) {
      foreach (element_children($empty_element[$delta]) as $col) {
        if (strlen($preset[$delta][$col])) {
          $element['preset'][$delta][$col]['#default_value'] = $preset[$delta][$col];
        }
      }
    }
  }
  else {//this is a normal field
    //the payer, payee, secondperson, direction, fields are required and cannot be set
    $element['#description'] = t("This field is required");
    $element['preset']['#default_value'] = $preset;
  }
  $element['strip'] = array(
    '#title' => t('Strip theme container'),
    '#description' => t("Disable the outer container, so it doesn't render the field #title and #description but gives you fuller control in the template below."),
    '#type' => 'checkbox',
    '#default_value' => @$field_defaults['strip'],
    '#weight' => 10
  );
  return $element;
}

function mcapi_forms_ajax_settings($form, &$form_state) {
  return $form;
}

/*
 * form validation callback for edit_mcapi_form
 */
function mcapi_forms_template_validate($form, &$form_state) {
  $values = &$form_state['values'];
  if (empty($values['experience']['template'])) return;
  //check that the critical transaction fields are either present in the template OR preset
  $required_fields = array('worth');
  $required_fields += $values['perspective'] == 1 ? array('secondperson', 'direction') : array('payer', 'payee');
  foreach ($required_fields as $fieldname) {
    if (empty($values[$fieldname]['preset']) && strpos($values['experience']['template'], "[mcapiform:$fieldname]") === FALSE) {
      form_set_error('template', t('Either preset @tokenname, or include that token in the template.',
        array('@tokenname' => "[mcapiform:$fieldname]")
      ));
    }
  }
}

function mcapi_forms_exists($machine_name, $element, &$form_state) {
  $forms = mcapi_forms_load();
  return isset($forms['machine_name']);
}


/*
 * validate callback for edit form,
 * declared in the ctools plugin $this->plugin['form']['submit'] in mcapi_forms_ui.inc
 * the $form_state[item] is the object to be saved, derived from $form_state[values]
 */
function mcapi_edit_mcapi_form_validate($form, &$form_state) {
  form_state_values_clean($form_state);
  //we need to start with an existing ctools object which contains essential default properties
  if (empty($form_state['item'])) {
    $form_state['item'] = ctools_export_crud_new('mcapi_forms');
  }

  $form_state['item']->name = $form_state['values']['info']['name'];//because this isn't editable at the moment
  unset($form_state['values']['name']);
  unset($form_state['values']['mcapi_form_fields']);
  //strip the leading slash just to be sure
  if (substr($form_state['values']['architecture']['path'], 0, 1) == '/') {
    $form_state['values']['architecture']['path'] = substr($form_state['values']['architecture']['path'], 1);
  }

  $form_state['item']->data = $form_state['values'];
  $mcapiform = &$form_state['item']->data;
  //need to check that all required fields exist in the form.
  if (!empty($mcapiform['experience']['template'])) {
    foreach ($mcapiform as $name => $prop) {
      if (!empty($prop['required']) && empty($prop['preset'])) {
        if (strpos($mcapiform['experience']['template'], $name) == FALSE) {
          form_error($form['experience']['template'], t("Required field '@name' must be used in the form template", array('@name' => $name)));
        }
      }
    }
  }
  cache_clear_all('mcapi_forms', 'cache');
}

/*
 * element validate callback for worth field with multiple cardinality
 * Ensure that null values are saved as NULL and not zero
 */
function mcapi_form_validate_worth_preset(&$element, &$form_state) {
  //if any of the currencies has been preset with a value, including 0 remove the empty ones before saving
  $items = &$form_state['values']['worth']['preset'];
  foreach ($items as $delta => $item) {
    if ($item['main_quant'] == '' && empty($item['div_quant'])) {
      //that means it won't be saved and will come in as blank next time
      unset($items[$delta]);
    }
  }
  form_set_value($element, $items, $form_state);
}

function _mcapi_user_chooser_config_field($default = array('user_chooser_config' => 'mcapi_access_check_permissions:transact')) {
  return array(
    '#title' => t('Users to choose from'),
    '#description' => t('Configure this widget more at !link',
       array('!link' => l('admin/config/people/user_chooser', 'admin/config/people/user_chooser', array('attributes'=>array('target'=> '_blank')))
       )),
    '#type' => 'select',
    '#options' => _mcapi_user_chooser_access_options(),
    '#default_value' => $default,
    '#required' => FALSE
  );
}

/*
 * this function provides options to configure the user_chooser widget for any one role or permission.
 * Other groups could be provided
 * This functionality is also implies that that there will be access callbacks to take these parameters.
 * But that will have to wait for another time and a more powerful utility module.
 * In the mean time, the access callbacks are in user_chooser
 */
function _mcapi_user_chooser_access_options() {
  static $user_chooser_options;
  if (!isset($user_chooser_options)) {
    $roles = user_roles(TRUE);
    foreach ($roles as $rid => $name) {
      $user_chooser_options[t('Roles')]['user_chooser_roles:'.$rid] = $name;
    }
    $permissions = module_invoke_all('permission');
    foreach ($permissions as $perm => $info) {
      $user_chooser_options[t('Permissions')]['user_chooser_permissions:'.$perm] = strip_tags($info['title']);
    }
  }
  return $user_chooser_options;
}





function _mcapi_direction_config_fields($defaults) {
  $defaults += array(
    'incoming' => t('Request'),
    'outgoing' => t('Payment'),
    'widget' => 'radios'
  );

  return array(
    'widget' => array(
      '#title' => t('Widget'),
      '#type' => 'radios',
      '#options' => array(
        'select' => t('Dropdown select box'),
        'radios' => t('Radio buttons')
      ),
      '#default_value' => $defaults['widget'],
      '#weight' => 1
    ),
    'incoming' => array(
      '#title' => t("'incoming' option label"),
      '#type' => 'textfield',
      '#default_value' => $defaults['incoming'],
      '#weight' => 2
    ),
    'outgoing' => array(
      '#title' => t("'outgoing' option label"),
      '#type' => 'textfield',
      '#default_value' => $defaults['outgoing'],
      '#weight' => 3
    )
  );
  return $element;
}