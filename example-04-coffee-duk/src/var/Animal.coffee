behave class Animal
  constructor: ->
    @name = 'Nite-Nite'

  onAttach: ->
    print @name + ' is alert'

  move: (meters) ->
    print @name + " moved #{meters}m."
