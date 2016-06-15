behave class Cherished
  constructor: ->
    @name = 'EB-Nite'

  onAttach: ->
    print @name + ' is alert'

  bump: ->
    print 'I am bump'

  move: (meters) ->
    print @name + " moved #{meters}m."
