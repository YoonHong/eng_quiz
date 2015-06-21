<?php

function _eng_quiz_pc_sub_list( $mid ) {

  $main_entity = _eng_quiz_get_single_entity( 'eng_quiz_main' , $mid );
  return drupal_get_form('_eng_quiz_sub_list_form', $main_entity);
}

function _eng_quiz_pc_sub_edit($mid, $sid) {

  $qmain = _eng_quiz_get_single_entity( 'eng_quiz_main' , $mid );
  $qsub = _eng_quiz_get_single_entity( 'eng_quiz_sub' , $sid );

  return drupal_get_form('_eng_quiz_sub_list_form', $qmain, $qsub);
}

function _eng_quiz_pc_sub_del($mid, $sid) {

	db_delete('eng_quiz_sub')
		->condition('id', $sid)
		->execute();

    db_delete('eng_quiz_sub_opt')
  		->condition('sid', $sid)
  		->execute();

	return _eng_quiz_pc_sub_list($mid);

}

function _eng_quiz_sub_list_form($form, &$form_state, $qmain = null, $qsub = null) {

  if( $qsub == null ) {
    $isUpdate = false;
    $formNoDisp = false;

  } else {
    $isUpdate = true;
    $formNoDisp = true;

  }

  $form['v_main_quiz'] = array(
    '#type' => 'value',
    '#value' => $qmain,
  );

  $form['v_sid'] = array(
    '#type' => 'value',
    '#value' => isset($qsub->id) ? $qsub->id : null,
  );

  $form['mQuiz'] = array(
    '#type' => 'markup',
    '#markup' => t(
          ' <p>Title : @title
            <br/>Type : @type</p>
          ',
           array('@title' => $qmain->title, '@type' => $qmain->type, )
    ),
  );

  $sList = _eng_quiz_get_sub_list( $qmain->id );
  $rows = array();

  foreach ($sList as $entity) {
    // Create tabular rows for our entities.
    if ( $entity->type == 2 ) {
      $eType = '주관식';
    } else {
      $eType = '객관식';
    }

    $rows[] = array(
      'data' => array(
      //  'id' => $entity->id,
      //  'item_description' => l($entity->title, 'test_quiz/test/' . $entity->id),
      'title' => t('@title', array('@title'=>$entity->title)),
        'type' => $eType,
        'edit_del' =>

          t('!E / !D',
            array('!E' => l(t('Edit'), 'eng_quiz/sub/'.$qmain->id.'/edit/'.$entity->id),
            '!D' => l(t('Del'), 'eng_quiz/sub/'.$qmain->id.'/del/'.$entity->id), )
          ),

      ),
    );
  }



  $form['qlist'] = array(
    '#prefix' => '<div id="qSubList">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
    '#theme' => 'table',
    '#header' => array(t('Question?'), t('Type'), t('Edit / Del')),
    '#rows' => $rows,
  );

  if ( $isUpdate ) {
    $dft_qType = $qsub->type;
  } else {
    $dft_qType = 1;
  }

  $form['question_type'] = array(
    '#title' => t('Question Type?'),
    '#type' => 'select',
    '#options' => array(1 => 'Select...', 2 =>'주관식', 3 => '객관식', ),
    '#default_value' => $dft_qType,
    '#disabled' => $formNoDisp,
    '#ajax' => array(
      'callback' => '_eng_quiz_ajax_add_question_callback',
      'wrapper' => 'questions_wrapper',
    ),

  );

  $form['questions_fieldset'] = array(
    '#type' => 'fieldset',
    // These provide the wrapper referred to in #ajax['wrapper'] above.
    '#prefix' => '<div id="questions_wrapper">',
    '#suffix' => '</div>',
  );

  if ( $isUpdate || !empty($form_state['values']['question_type'])) {
    if ( $isUpdate ) {
      $vQType = $qsub->type;
    } else {
      $vQType = $form_state['values']['question_type'];
    }

    if ( $vQType != 1 ) {
      // Empty

      $form['questions_fieldset']['question'] = array(
        '#type' => 'textfield',
        '#title' => t('Question :'),
        '#default_value' => isset($qsub->title) ? $qsub->title : '',
        '#required' => TRUE,
      );

      if ( $vQType == 2 ) {
        $form['questions_fieldset']['answer'] = array(
          '#type' => 'textfield',
          '#title' => t('Answer :'),
          '#default_value' => isset($qsub->answer) ? $qsub->answer : '',
          '#required' => TRUE,
        );

      } else if ( $vQType == 3 ) {
          if ( $isUpdate ) {
            $dftOptCnt = $qsub->opt_cnt;
            $dftAns = $qsub->answer;
          } else {
            $dftOptCnt = 4;
            $dftAns = 0;
          }


            $form['questions_fieldset']['opt_cnt'] = array(
              '#title' => t('How many options?'),
              '#type' => 'select',
              '#options' => array(1 => 1, 2 => 2, 3 => 3, 4 => 4, 5 => 5, 6 => 6, 7 => 7,),
              '#default_value' => $dftOptCnt,
              '#disabled' => $formNoDisp,
              '#ajax' => array(
                'callback' => '_eng_quiz_ajax_add_option_callback',
                'wrapper' => 'options_wrapper',
              ),

            );


            $form['questions_fieldset']['box'] = array(
              '#type' => 'fieldset',
              // These provide the wrapper referred to in #ajax['wrapper'] above.
              '#prefix' => '<div id="options_wrapper">',
              '#suffix' => '</div>',
            );


            if (empty($form_state['values']['opt_cnt'])) {
              $optCnt = $dftOptCnt;
            } else {
              $optCnt = $form_state['values']['opt_cnt'];
            }

            $arrOptAns[0] = t('Select...');
            for ($i = 1; $i <= $optCnt; $i++) {
              $arrOptAns[$i] = t('Option @ans', array('@ans'=>$i,));
            }

            $form['questions_fieldset']['box']['answer'] = array(
              '#title' => t('Answer :'),
              '#type' => 'select',
              //'#options' => array(1 => 'Select...',),
              '#options' => $arrOptAns,
              '#default_value' => $dftAns,
            );

            if ( isset( $qsub ) ) {
              $opts = _eng_quiz_get_opt_list( $qsub->id );

              foreach($opts as $opt) {
                $form['questions_fieldset']['box']['opt'.$opt->seq] = array(
                  '#type' => 'textfield',
                  '#title' => t('Option @num', array('@num'=>$opt->seq,)),
                  '#default_value' => $opt->title,
                );

              }

            } else {

              for ($i = 1; $i <= $optCnt; $i++) {
                $form['questions_fieldset']['box']['opt'.$i] = array(
                  '#type' => 'textfield',
                  '#title' => t('Option @num', array('@num'=>$i,)),
                );
              }
            }
      }
    }
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $isUpdate ? t('Update Question!') : t('Save Question!'),
    '#weight' => 50,
  );

  return $form;
}

function _eng_quiz_ajax_add_question_callback($form, $form_state) {
  $form['questions_fieldset']['box']['answerS']['#value'] = '';
  return $form['questions_fieldset'];
}

function _eng_quiz_ajax_add_option_callback($form, $form_state) {
  $form['questions_fieldset']['box']['answerO']['#value'] = 0;
  return $form['questions_fieldset']['box'];
}

function _eng_quiz_sub_list_form_validate($form, &$form_state) {
  //dpm( format_date(REQUEST_TIME, 'custom', 'Y-m-d' ) ) ;
  $v_qtype = $form_state['values']['question_type'];

  if ( $v_qtype == 1 ) {
    form_set_error('title', t('Select Question Type!!!'));
  }
}

function _eng_quiz_sub_list_form_submit($form, &$form_state) {
  $v_main_quiz = $form_state['values']['v_main_quiz'];
  $mid = $v_main_quiz->id;
  $sid = $form_state['values']['v_sid'];
  $qtype = $form_state['values']['question_type'];

  if(isset($sid)) {
    $entity = _eng_quiz_get_single_entity('eng_quiz_sub', $sid);
  }else{
    $entity = entity_create('eng_quiz_sub', array('id' => null));
  }

  $entity->title  = $form_state['values']['question'];
  $entity->type  = $qtype;
  $entity->answer  = $form_state['values']['answer'];
  $entity->mid  = $mid;

  if( $qtype == 3 ) {
    // 객관식
    $entity->opt_cnt  = $form_state['values']['opt_cnt'];;
  }

  if( $entity->id == null ) {
  	// Insert

    drupal_write_record('eng_quiz_sub', $entity);

    if( $qtype == 3 ) {
      // 객관식
      $cnt = $form_state['values']['opt_cnt'];

      for( $i = 1; $i <= $cnt; $i++ ) {
        $ent_opt = entity_create('eng_quiz_sub_opt', array('id' => null));
        $ent_opt->sid = $entity->id;
        $ent_opt->seq = $i;
        $ent_opt->title = $form_state['values']['opt'.$i];

        drupal_write_record('eng_quiz_sub_opt', $ent_opt);
      }

    }

  } else {
    // Edit
  	drupal_write_record('eng_quiz_sub', $entity, 'id');

    if( $qtype == 3 ) {

      // 객관식
      $opts = _eng_quiz_get_opt_list( $entity->id );

      foreach($opts as $opt) {
        $opt->title = $form_state['values']['opt'.$opt->seq];
        drupal_write_record('eng_quiz_sub_opt', $opt, 'id');
      }

    }

  }



  $form_state['redirect'] = 'eng_quiz/sub/'.$mid;
}
