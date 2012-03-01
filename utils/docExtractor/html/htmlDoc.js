$().ready(
	function($) {
		// Active folding/unfolding
		$('.expander').click(
			function(event) {
				event.preventDefault();
				$(this).siblings('.expandable').toggle();
			}
			);
		
		// Fold everything
		$('.expandable').hide();
	}
);
