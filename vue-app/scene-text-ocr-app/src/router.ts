import Vue from 'vue';
import Router from 'vue-router';
import SceneTextOCR from './views/SceneTextOCR.vue';

Vue.use(Router);

export default new Router({
  routes: [
    {
      path: '/',
      name: 'SceneTextOCR',
      component: SceneTextOCR,
    },
  ],
});
