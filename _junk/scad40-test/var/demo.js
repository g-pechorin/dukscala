
function Hamster ()
{
  this.onStart = function()
  {
    peterlavalle.magpie.System.out('I am up!')
  }
  this.onUpdate = function()
  {
    peterlavalle.magpie.System.out('I am update ' + peterlavalle.magpie.Time.now + "\n\t" + peterlavalle.magpie.Time.delta)
  }
  this.onClose = function()
  {
    peterlavalle.magpie.System.out('I am closed')
  }
}
