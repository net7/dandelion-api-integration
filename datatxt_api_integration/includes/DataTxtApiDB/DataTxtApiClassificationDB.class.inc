<?php

class DataTxtApiClassificationDB extends DataTxtApiDB {

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    $this->table_vocabulary = 'datatxt_taxonomy_vocabulary';
    $this->table_term = 'datatxt_taxonomy_term';
    $this->isClassification = TRUE;
    $this->enabledVocabularyVariable = 'datatxt_classification_vocabulary';
    $this->modelUpdateTimeVariable = 'datatxt_classification_model_update';
    $this->minScoreVariable = 'datatxt_classification_min_score';
    $this->minScoreMultiplier = floatval(DATATXT_CLASSIFICATION_SCORE_MULTIPLIER);
    $this->wikiSearchCallback = 'callback_datatxt_api_integration_classification_wikisearch';
  }

  /**
   * {@inheritdoc}
   */
  public static function getInstance() {
    if (empty(self::$instance) || get_class(self::$instance) != 'DataTxtApiClassificationDB') {
      self::$instance = new DataTxtApiClassificationDB();
    }
    return self::$instance;
  }

  /**
   * {@inheritdoc}
   */
  public function getConcepts($content, $langcode) {
    $taxonomy_vocabulary = $this->getEnabledVocabulary($langcode);
    if (!empty($taxonomy_vocabulary)) {
      $min_score = $this->getAPIMinScoreVar();
      $service = DataTxtClassificationService::getInstance();
      $vocabulary = $this->loadVocabulary($taxonomy_vocabulary->vid, $langcode);
      if (!empty($vocabulary)) {
        $result = $service->classifyHtmlFragmentContent($content, $vocabulary->datatxt_id, $min_score);
        return(sizeof($result) > 0) ? $result : FALSE;
      }
    }
    return FALSE;
  }

}

/**
 * Callback for WikiSearch
 *
 * Calls instance's wikiSearchCallback method
 *
 * @param &$form
 *   The form array; this is a reference so the caller can get back
 *   whatever information the form(s) involved left for it.
 * @param &$form_state
 *   The form state array; this is a reference so the caller can get back
 *   whatever information the form(s) involved left for it.
 *
 * @ingroup callbacks
 */
function callback_datatxt_api_integration_classification_wikisearch(&$form, &$form_state) {
  $api = DataTxtApiClassificationDB::getInstance();
  return $api->wikiSearchCallback($form, $form_state);
}
