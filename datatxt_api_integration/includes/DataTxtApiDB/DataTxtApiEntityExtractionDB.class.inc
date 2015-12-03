<?php

class DataTxtApiEntityExtractionDB extends DataTxtApiDB {

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    $this->table_vocabulary = 'datatxt_entity_vocabulary';
    $this->table_term = 'datatxt_entity_term';
    $this->isEntityExtraction = TRUE;
    $this->enabledVocabularyVariable = 'datatxt_entity_extraction_vocabulary';
    $this->minScoreVariable = 'datatxt_entity_extraction_min_score';
    $this->minScoreMultiplier = floatval(DATATXT_ENTITY_EXTRACTION_SCORE_MULTIPLIER);
  }

  /**
   * {@inheritdoc}
   */
  public static function getInstance() {
    if (empty(self::$instance) || get_class(self::$instance) != 'DataTxtApiEntityExtractionDB') {
      self::$instance = new DataTxtApiEntityExtractionDB();
    }
    return self::$instance;
  }

  /**
   * {@inheritdoc}
   */
  public function saveModelToDataTxt($vid, $langcode, $debug = FALSE, $display = FALSE, $force = FALSE) {
    return FALSE;
  }

  /**
   * {@inheritdoc}
   */
  public function sendModelsToDataTxt($isCron = FALSE) {
    return FALSE;
  }

  /**
   * {@inheritdoc}
   */
  public function getConcepts($content, $langcode) {
    $min_score = $this->getAPIMinScoreVar();
    $service = DataTxtEntityExtractionService::getInstance();
    $result = $service->classifyTextContent($content, $langcode, $min_score);
    return(sizeof($result) > 0) ? $result : FALSE;
  }

}
