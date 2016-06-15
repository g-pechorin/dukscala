print 'Hello'

Log = underlay.Log

Log.warn 'Caw Caw!'

Log.info "... okay ; I'm ready to rock"

behave class
  onAttach: ->
    Log.info 'I am a bird!'
  onDetach: ->
    Log.info 'I am a rock!'
