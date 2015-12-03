(function ($) {

  Drupal.behaviors.dataTXTApiIntegrationAdminTopicAddTopic = {

    attach: function (context, settings) {
      if(context == document){ // Prevents multiple calls, faster than once method
        $('.datatxt-term-admin .action-delete').click(function () {
          var $wrapper = $(this).closest('.topic.active');
          $('.var', $wrapper).html(settings.datatxt.title_new);
          $('input[type=text], input[type=hidden]', $wrapper).val('');
          $('select', $wrapper).val('0.5');
          $('.form-item-result > div', $wrapper).html('');      
          $wrapper.removeClass('active');
          $wrapper.parent().find('a.add').show();
        });
        $('.datatxt-term-admin a.add').click(function () {
          var usedItems = $('.datatxt-term-admin .topic.active');
          var cleanItems = $('.datatxt-term-admin .topic').not('.active');
          var usedCount = usedItems.size();
          var cleanCount = cleanItems.size();
          if (cleanCount > 0) {
            var item = $(cleanItems.get(0));
            $('> fieldset.collapsible',item).removeClass('collapsed');
            $('> fieldset.collapsible .fieldset-wrapper',item).show();     
            if(usedCount>0){
              var lastUsed = $(usedItems.get(usedCount-1));
              lastUsed.after(item);
            }
            item.addClass('active');
            item.find('.form-text').focus();
            item.find('.form-text').bind('keypress', function(e) {
              if (e.keyCode === 13) {
                item.find('.form-item-search-btn button').trigger('mousedown');
              }
            });
          }
          if (cleanCount < 2) {
            $(this).hide();
          }
        });
        $('.datatxt-term-admin .url-item').on('blur', function(){
          $(this).closest('.topic').find('.var').html($(this).val() + ' (*)');
        });
      }
    }
  };

  $.fn.extend({
    topics: function (index) {
      var wrapper = $('.datatxt-term-admin #edit-topic-'+index);
      var lnk = $('.form-item-result .item a.title', wrapper);
      $(lnk).click(function(){
        $('.tip').hide();
        var group = $(this).closest('div.item');
        $('.selected', wrapper).removeClass('selected');
        group.addClass('selected');
        var title = $('.uri', group).text();
        $('#edit-url-'+index).val(title);
        // Filter title.
        title = title.replace(/http:\/\/[a-z]{2}\.wikipedia\.org\/wiki\//, '');
        title = decodeURI(title.replace('_', ' '));
        $('#edit-topic-'+index+' .var').html(title + ' <span class="new-item">(*)</span>');
        $('#edit-topic-'+index).addClass('collapsed');
        $('#edit-topic-'+index+' .fieldset-wrapper').hide(); 
      });
      $('.form-item-result .item', wrapper).mouseenter(function(){
        $('.tip').hide();
        $('.tip', $(this))
        .fadeIn()
        .position({my: 'bottom right'});
      })
      .mouseleave(function(){
        $('.tip').hide();
      });
    }
});

})(jQuery);
