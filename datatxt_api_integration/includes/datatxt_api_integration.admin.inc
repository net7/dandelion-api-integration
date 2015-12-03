<?php

/**
 * Created by PhpStorm.
 * User: max2thousand
 * Date: 24/03/15
 * Time: 11:49
 */

/**
 * Admin settings credentials callback.
 */
function datatxt_api_integration_admin_settings_credentials($form, $form_state) {

  $form['datatxt_api_endpoint'] = array(
    '#type' => 'textfield',
    '#title' => t('DataTxt API endpoint'),
    '#description' => t('DataTxt API endpoint'),
    '#default_value' => variable_get('datatxt_api_endpoint', ''),
    '#attributes' => array('disabled' => 'disabled'),
  );

  $form['datatxt_app_id'] = array(
    '#type' => 'textfield',
    '#title' => t('DataTxt App ID'),
    '#description' => t('DataTxt App ID'),
    '#default_value' => variable_get('datatxt_app_id', ''),
  );

  $form['datatxt_app_key'] = array(
    '#type' => 'textfield',
    '#title' => t('DataTxt App key'),
    '#description' => t('DataTxt App key'),
    '#default_value' => variable_get('datatxt_app_key', ''),
  );

  return system_settings_form($form);
}

/**
 * Admin settings preferences callback.
 */
function datatxt_api_integration_admin_settings_preferences($form, $form_state) {

  drupal_add_css(drupal_get_path('module', 'datatxt_api_integration') . '/css/admin.css');
  $articleVocabularies = taxonomy_get_vocabularies('article');
  $dataTxtApiClassificationDB = DataTxtApiClassificationDB::getInstance();
  $dataTxtApiEntityExtractionDB = DataTxtApiEntityExtractionDB::getInstance();
  $dataTxtApiUserInterestsDB = DataTxtApiUserInterestsDB::getInstance();

  $classification_options = array(t('Select vocabulary'));
  $classification_selection = $dataTxtApiClassificationDB->getEnabledVocabularyVid();
  foreach ($articleVocabularies as $vid => $vocabulary) {
    if ($dataTxtApiClassificationDB->isDataTxt($vid)) {
      $classification_options[$vid] = $vocabulary->name;
    }
  }

  $entity_extraction_options = array(t('Select vocabulary'));
  $entity_extraction_selection = $dataTxtApiEntityExtractionDB->getEnabledVocabularyVid();
  foreach ($articleVocabularies as $vid => $vocabulary) {
    if ($dataTxtApiEntityExtractionDB->isDataTxt($vid)) {
      $name = $dataTxtApiEntityExtractionDB->getVocabularyName($vid);
      $entity_extraction_options[$vid] = $vocabulary->name;
    }
  }

  $user_interests_options = array(t('Select vocabulary'));
  $user_interests_selection = $dataTxtApiUserInterestsDB->getEnabledVocabularyVid();
  foreach ($articleVocabularies as $vid => $vocabulary) {
    if ($dataTxtApiUserInterestsDB->isDataTxt($vid)) {
      $user_interests_options[$vid] = $vocabulary->name;
    }
  }

  $sendOptions = array(
    0 => t('Disabled'),
    '3600' => t('Every hours'),
    '14400' => t('Every 4 hours'),
    '28800' => t('Every 8 hours'),
    '43200' => t('Every 12 hours'),
    '86400' => t('Every days'),
    '604800' => t('Every weeks'),
  );


  $form['datatxt_classification'] = array(
    '#type' => 'fieldset',
    '#title' => t('Classification'),
    '#prefix' => '<div class="datatxt-preferences-admin">',
    '#suffix' => '</div>',
  );

  $form['datatxt_classification']['datatxt_classification_vocabulary'] = array(
    '#type' => 'select',
    '#title' => t('Assigned vocabulariy'),
    '#description' => t('Vocabulary assigned to classification'),
    '#options' => $classification_options,
    '#default_value' => $classification_selection,
    '#attributes' => array('id' => 'edit-datatxt-classification-vocabulary')
  );

  $form['datatxt_classification']['datatxt_classification_model_update'] = array(
    '#type' => 'select',
    '#required' => FALSE,
    '#title' => t('Classification Auto send model'),
    '#options' => $sendOptions,
    '#description' => t('Choose how often to send a modified classification vocabulary to DataTXT.'),
    '#default_value' => $dataTxtApiClassificationDB->getModelUpdateTime(),
    '#states' => array(
      'invisible' => array(
        '#edit-datatxt-classification-vocabulary' => array('value' => 0)
      )
    )
  );

  $form['datatxt_classification']['datatxt_user_interests_vocabulary'] = array(
    '#type' => 'select',
    '#title' => t('Assigned vocabulariy'),
    '#description' => t('Container vocabulary for users\' vocabolaries'),
    '#options' => $user_interests_options,
    '#default_value' => $user_interests_selection,
    '#attributes' => array('id' => 'edit-datatxt-user-interests-vocabulary')
  );

  $form['datatxt_classification']['datatxt_user_interest_model_update'] = array(
    '#type' => 'select',
    '#required' => FALSE,
    '#title' => t('User Interests Auto send model'),
    '#options' => $sendOptions,
    '#description' => t('Choose how often to send a modified user interests vocabulary to DataTXT.'),
    '#default_value' => $dataTxtApiUserInterestsDB->getModelUpdateTime(),
    '#states' => array(
      'invisible' => array(
        '#edit-datatxt-user-interests-vocabulary' => array('value' => 0)
      )
    )
  );

  $form['datatxt_classification']['datatxt_classification_min_score'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#number_type' => 'decimal',
    '#title' => t('Min. score'),
    '#description' => t('Insert a decimal value between 0 and 1'),
    '#default_value' => $dataTxtApiClassificationDB->getMinScoreVar(),
  );

  $form['datatxt_entity_extraction'] = array(
    '#type' => 'fieldset',
    '#title' => t('Entity Extraction'),
    '#prefix' => '<div class="datatxt-preferences-admin">',
    '#suffix' => '</div>',
  );

  $form['datatxt_entity_extraction']['datatxt_entity_extraction_vocabulary'] = array(
    '#type' => 'select',
    '#title' => t('Assigned vocabulary'),
    '#description' => t('Vocabulary assigned to entity extraction'),
    '#options' => $entity_extraction_options,
    '#default_value' => $entity_extraction_selection,
  );

  $form['datatxt_entity_extraction']['datatxt_entity_extraction_min_score'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#number_type' => 'decimal',
    '#title' => t('Min. score'),
    '#description' => t('Insert a decimal value between 0 and 1'),
    '#default_value' => $dataTxtApiEntityExtractionDB->getMinScoreVar(),
  );

  $form['datatxt_debug_mode'] = array(
    '#type' => 'checkbox',
    '#title' => t('dataTXT Debug Mode'),
    '#default_value' => variable_get('datatxt_debug_mode', FALSE),
  );

  return system_settings_form($form);
}

function datatxt_api_integration_admin_settings_preferences_validate($form, $form_state) {

  $classification_value = floatval($form_state['values']['datatxt_classification_min_score']);
  if ($classification_value < 0 || $classification_value > 1) {
    form_set_error('datatxt_classification_min_score', t('You must enter a decimal value between 0 and 1'));
  }
  $entity_extraction_value = floatval($form_state['values']['datatxt_entity_extraction_min_score']);
  if ($entity_extraction_value < 0 || $entity_extraction_value > 1) {
    form_set_error('datatxt_entity_extraction_min_score', t('You must enter a decimal value between 0 and 1'));
  }

  $c1 = $form_state['values']['datatxt_classification_vocabulary'];
  $c2 = $form_state['values']['datatxt_entity_extraction_vocabulary'];
  $c3 = $form_state['values']['datatxt_user_interests_vocabulary'];

  if (($c1 == $c2) || ($c1 == $c3) || ($c3 == $c2)) {
    form_set_error('datatxt_classification_vocabulary', t('Vocabularies be assigned to one DataTXT feature only'));
  }
}
