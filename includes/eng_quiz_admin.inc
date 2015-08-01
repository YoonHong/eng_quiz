<?php

function _eng_quiz_pc_admin_quiz_list() {

  $content['preface']['#markup'] = t( 'Student Quiz List' );

  $content['table'] = _eng_quiz_admin_ql_table();

  return $content;
}

function _eng_quiz_admin_ql_table() {
  $content = array();

  $user = $GLOBALS['user'];
  $uid = $user->uid;

  $query = db_select('eng_quiz_student_qlist', 'sq')->extend('PagerDefault');
  $query->join('eng_quiz_main', 'mq', 'mq.id = sq.mid');
  $query->join('users', 'user', 'user.uid = sq.uid');

  $query->fields('mq',array('title', 'type'))
        ->fields('user',array('name'))
        ->fields('sq',array('id', 'mid', 'uid', 'cnt', 'created'));
  $query->limit(20)
        ->orderBy('sq.created', 'DESC');

  $result = $query->execute();

  if ($result->rowCount() > 0 ) {

    foreach ($result as $entity) {
      // Get Cnt of Sub Quiz]
      $total_cnt = db_query(
          'SELECT COUNT(*)
            FROM {eng_quiz_sub} A
            WHERE 1=1
            AND A.mid = :mid', array(':mid' => $entity->mid)
          )->fetchField();

      // Create tabular rows for our entities.
      $rows[] = array(
        'data' => array(
          'uid' => $entity->name,
          'title' => l($entity->title, 'eng_quiz/student/'.$entity->id.'/qanswer/'.$uid),
          'cnt' => ''.$entity->cnt.' / '.$total_cnt,
          'created' => format_date( $entity->created, 'custom', 'Y-M-d' ),
        ),
      );
    }
    // Put our entities into a themed table. See theme_table() for details.
    $content['entity_table'] = array(
      '#theme' => 'table',
      '#rows' => $rows,
      '#header' => array(t('User'), t('Title'), t('Cnt'), t('Test Date')),
    );

    // Attach the pager theme.
    $content['entity_pager'] = array('#theme' => 'pager');
  }
  else {
    // There were no entities. Tell the user.
    $content[] = array(
      '#type' => 'item',
      '#markup' => t('No History!!!'),
    );
  }

  return $content;

}
